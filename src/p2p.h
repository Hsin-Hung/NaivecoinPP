#ifndef _P2P_H_
#define _P2P_H_

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
#include <pthread.h>

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

using websocketpp::lib::bind;

class broadcast_server
{
private:
    typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

    server m_server;
    con_list m_connections;

public:
    broadcast_server()
    {
        m_server.init_asio();

        m_server.set_open_handler(bind(&broadcast_server::on_open, this, ::_1));
        m_server.set_close_handler(bind(&broadcast_server::on_close, this, ::_1));
        m_server.set_message_handler(bind(&broadcast_server::on_message, this, ::_1, ::_2));
    }

    void on_open(connection_hdl hdl)
    {
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl)
    {
        m_connections.erase(hdl);
    }

    void on_message(connection_hdl hdl, server::message_ptr msg)
    {
        for (auto it : m_connections)
        {
            m_server.send(it, msg);
        }
    }

    void run(uint16_t port)
    {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
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

    void broadcastLatest()
    {

        con_list::iterator it;
        Block latestBlock = Chain::getInstance()->getLastestBlock();
        nlohmann::json j;
        to_json(j, latestBlock);
        for (it = m_connections.begin(); it != m_connections.end(); ++it)
        {

            m_server.send(*it, j.dump(), websocketpp::frame::opcode::text);
        }
    }
};

extern broadcast_server server_instance;

void *p2pServer(void *);

void on_message(client *c, websocketpp::connection_hdl hdl, message_ptr msg);

int connectToPeer(std::string uri);

#endif
