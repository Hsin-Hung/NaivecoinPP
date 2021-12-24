#include "block.h"
#include "chain.h"
#include <iostream>
#include <string>
#include "../include/httplib.h"
#include "../include/json.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/thread.hpp>
#include <set>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::connection_hdl;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

using websocketpp::lib::condition_variable;
using websocketpp::lib::lock_guard;
using websocketpp::lib::mutex;
using websocketpp::lib::thread;
using websocketpp::lib::unique_lock;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;
/* on_open insert connection_hdl into channel
 * on_close remove connection_hdl from channel
 * on_message queue send to all channels
 */

enum action_type
{
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE
};

struct action
{
    action(action_type t, connection_hdl h) : type(t), hdl(h) {}
    action(action_type t, connection_hdl h, server::message_ptr m)
        : type(t), hdl(h), msg(m) {}

    action_type type;
    websocketpp::connection_hdl hdl;
    server::message_ptr msg;
};

class broadcast_server
{
public:
    broadcast_server()
    {
        // Initialize Asio Transport
        m_server.init_asio();

        // Register handler callbacks
        m_server.set_open_handler(bind(&broadcast_server::on_open, this, ::_1));
        m_server.set_close_handler(bind(&broadcast_server::on_close, this, ::_1));
        m_server.set_message_handler(bind(&broadcast_server::on_message, this, ::_1, ::_2));
    }

    void run(uint16_t port)
    {
        // listen on specified port
        m_server.listen(port);

        // Start the server accept loop
        m_server.start_accept();

        // Start the ASIO io_service run loop
        try
        {
            m_server.run();
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    void on_open(connection_hdl hdl)
    {
        {
            lock_guard<mutex> guard(m_action_lock);
            // std::cout << "on_open" << std::endl;
            m_actions.push(action(SUBSCRIBE, hdl));
        }
        m_action_cond.notify_one();
    }

    void on_close(connection_hdl hdl)
    {
        {
            lock_guard<mutex> guard(m_action_lock);
            // std::cout << "on_close" << std::endl;
            m_actions.push(action(UNSUBSCRIBE, hdl));
        }
        m_action_cond.notify_one();
    }

    void on_message(connection_hdl hdl, server::message_ptr msg)
    {
        // queue message up for sending by processing thread
        {
            lock_guard<mutex> guard(m_action_lock);
            // std::cout << "on_message" << std::endl;
            m_actions.push(action(MESSAGE, hdl, msg));
        }
        m_action_cond.notify_one();
    }

    void process_messages()
    {
        while (1)
        {
            unique_lock<mutex> lock(m_action_lock);

            while (m_actions.empty())
            {
                m_action_cond.wait(lock);
            }

            action a = m_actions.front();
            m_actions.pop();

            lock.unlock();

            if (a.type == SUBSCRIBE)
            {
                lock_guard<mutex> guard(m_connection_lock);
                m_connections.insert(a.hdl);
            }
            else if (a.type == UNSUBSCRIBE)
            {
                lock_guard<mutex> guard(m_connection_lock);
                m_connections.erase(a.hdl);
            }
            else if (a.type == MESSAGE)
            {
                lock_guard<mutex> guard(m_connection_lock);

                con_list::iterator it;
                for (it = m_connections.begin(); it != m_connections.end(); ++it)
                {
                    m_server.send(*it, a.msg);
                }
            }
            else
            {
                // undefined.
            }
        }
    }

    con_list get_connections()
    {
        return m_connections;
    }

    std::string get_connections_str()
    {
        std::string str;
        for (auto h : m_connections)
        {
            server::connection_ptr con = m_server.get_con_from_hdl(h);
            str += con->get_remote_endpoint() + "\n";
        }
        return str;
    }

    void add_new_connection(connection_hdl hdl)
    {
        m_connections.insert(hdl);
    }

private:
    server m_server;
    std::queue<action> m_actions;
    con_list m_connections;
    mutex m_action_lock;
    mutex m_connection_lock;
    condition_variable m_action_cond;
};

broadcast_server server_instance;

int p2pServer()
{
    try
    {
        server_instance;

        // Start a thread to run the processing loop
        thread t(bind(&broadcast_server::process_messages, &server_instance));

        // Run the asio loop with the main thread
        server_instance.run(9002);

        t.join();
    }
    catch (websocketpp::exception const &e)
    {
        std::cout << e.what() << std::endl;
    }
}

void on_message(client *c, websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    websocketpp::lib::error_code ec;

    c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
    if (ec)
    {
        std::cout << "Echo failed because: " << ec.message() << std::endl;
    }
}

int connectToPeer(std::string uri)
{

    client c;

    try
    {
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        c.init_asio();

        // Register our message handler
        c.set_message_handler(bind(&on_message, &c, ::_1, ::_2));

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec)
        {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        c.connect(con);

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        c.run();
    }
    catch (websocketpp::exception const &e)
    {
        std::cout << e.what() << std::endl;
    }
}

int main(int argc, char *argv[])
{

    p2pServer();

    httplib::Server svr;

    svr.Get("/blocks", [](const httplib::Request &, httplib::Response &res)
            {
                std::cout << "blocks " << std::endl;
                nlohmann::json j = Chain::getInstance()->getBlockChain();
                res.set_content(j.dump(), "application/json"); });

    svr.Post("/mineBlock", [](const httplib::Request &req, httplib::Response &res)
             {
                 std::cout << "mineBlock " << req.body << std::endl;
                 Block newBlock = generateNextBlock(req.body);
                 nlohmann::json j = newBlock;
                 res.set_content(j.dump(), "application/json"); });
    svr.Get("/peers", [](const httplib::Request &, httplib::Response &res)
            {
                std::cout << "peers " << std::endl;
                res.set_content(server_instance.get_connections_str(), "text/plain"); });
    svr.Post("/addPeer", [](const httplib::Request &req, httplib::Response &res)
             {

                 std::cout << req.body << std::endl;
                 connectToPeer(req.body);
                 res.set_content("add peer!", "text/plain"); });
    svr.listen("0.0.0.0", 8080);

    return 0;
}