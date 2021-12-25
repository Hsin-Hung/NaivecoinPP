#include "block.h"
#include "chain.h"
#include <iostream>
#include <string>
#include <pthread.h>
#include "p2p.h"
#include "../include/httplib.h"
#include "../include/json.hpp"

int main(int argc, char *argv[])
{
    pthread_t ptid;

    pthread_create(&ptid, NULL, &p2pServer, NULL);

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

    pthread_join(ptid, NULL);
    return 0;
}