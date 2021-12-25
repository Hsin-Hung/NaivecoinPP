#include "block.h"
#include "chain.h"
#include <iostream>
#include <sstream>
#include <string>
#include <pthread.h>
#include "p2p.h"
#include "../include/httplib.h"
#include "../include/json.hpp"

int main(int argc, char *argv[])
{

    // you can provide ip addresses for both http and websocket servers
    if (argc == 3)
    {

        std::stringstream a1(argv[1]), a2(argv[2]);
        a1 >> http_server_port;
        a2 >> socket_server_port;
        std::cout << http_server_port << " " << socket_server_port << std::endl;
    }

    pthread_t ptid;
    pthread_create(&ptid, NULL, &p2pServer, NULL);

    httplib::Server svr;

    svr.Get("/blocks", [](const httplib::Request &, httplib::Response &res)
            {
                std::cout << "Get blocks!" << std::endl;
                nlohmann::json j = Chain::getInstance()->getBlockChain();
                res.set_content(j.dump(), "application/json"); });

    svr.Post("/mineBlock", [](const httplib::Request &req, httplib::Response &res)
             {
                 std::cout << "Mine blocks!" << req.body << std::endl;
                 Block newBlock = generateNextBlock(req.body);
                 nlohmann::json j = newBlock;
                 res.set_content(j.dump(), "application/json"); });
    svr.Get("/peers", [](const httplib::Request &, httplib::Response &res)
            {
                std::cout << "Get peers!" << std::endl;
                res.set_content(server_instance.get_connections_str(), "text/plain"); });
    svr.Post("/addPeer", [](const httplib::Request &req, httplib::Response &res)
             {
                 std::cout << "Add peers!" << std::endl;
                 pthread_t ptid;
                 pthread_create(&ptid, NULL, connectToPeer, (void*) &req.body);
                 res.set_content("Add peer!", "text/plain"); });

    svr.listen("0.0.0.0", http_server_port);

    pthread_join(ptid, NULL);
    return 0;
}