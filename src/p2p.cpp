#include "p2p.h"

broadcast_server server_instance;
int http_server_port{8080};
int socket_server_port{8081};

void *p2pServer(void *)
{
    server_instance.run(socket_server_port);
}

// This message handler will be invoked once for each incoming message. It
// prints the message and then sends a copy of the message back to the server.
void on_message(client *c, websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    server_instance.on_message(hdl, msg);
}

void on_open(client *c, websocketpp::connection_hdl hdl)
{
    std::cout << "New connection added !" << std::endl;
    server_instance.add_new_connection(hdl);
    nlohmann::json j;
    j["type"] = QUERY_LATEST;
    c->send(hdl, j.dump(), websocketpp::frame::opcode::text);
}

void on_close(client *c, websocketpp::connection_hdl hdl)
{
    std::cout << "Connection closed" << std::endl;
}

void *connectToPeer(void *uri)
{
    client c;

    try
    {
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        c.init_asio();

        // Register our handlers
        c.set_open_handler(bind(&on_open, &c, ::_1));
        c.set_close_handler(bind(&on_close, &c, ::_1));
        c.set_message_handler(bind(&on_message, &c, ::_1, ::_2));

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(*static_cast<std::string *>(uri), ec);
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

    pthread_exit(NULL);
}