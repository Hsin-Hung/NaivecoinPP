#ifndef _CHAIN_H_
#define _CHAIN_H_

#include <vector>
#include "block.h"
#include "../include/json.hpp"

class Chain
{
private:
    Chain();
    static Chain *instance;
    std::vector<Block> blockChain;

public:
    static Chain *getInstance();

    Block &getLastestBlock();
    Chain(Chain &otherChain) = delete;
    void operator=(const Chain &) = delete;
    void replaceChain(std::vector<Block> newChain);
    bool isValidNewBlock(Block &newBlock, Block &prevBlock);
    bool isValidChain();
    bool addToChain(Block newBlock);
    void broadcastLatest();
    void broadcastQueryAll();
    std::vector<Block> getBlockChain();

};

void to_json(nlohmann::json& j, const Chain& c);
void from_json(const nlohmann::json& j, Chain& c);
void handleBlockchainResponse(std::vector<Block> blocks);
#endif