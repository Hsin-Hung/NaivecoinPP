#include "chain.h"
#include "poW.h"
#include "p2p.h"

Chain *Chain::instance = nullptr;

Chain::Chain() : blockChain{genesisBlock} {};

Chain *Chain::getInstance()
{

    if (instance == nullptr)
    {
        instance = new Chain();
    }

    return instance;
};

bool Chain::isValidNewBlock(Block &newBlock, Block &prevBlock)
{

    if (prevBlock.index + 1 != newBlock.index)
    {

        return false;
    }
    else if (prevBlock.hash != newBlock.prev_hash)
    {

        return false;
    }
    else if (newBlock.calBlockHash() != newBlock.hash)
    {
        return false;
    }
    else if (!isValidTimestamp(newBlock, prevBlock))
    {
        return false;
    }
    else if (!hasValidHash(newBlock))
    {
        return false;
    }

    return true;
}

bool Chain::isValidBlockStructure(Block &block)
{

    return true;
}

bool Chain::isValidChain()
{

    if (genesisBlock != blockChain.at(0))
    {
        return false;
    }

    for (int i = 1; i < blockChain.size(); ++i)
    {
        if (!isValidNewBlock(blockChain.at(i - 1), blockChain.at(i)))
        {
            return false;
        }
    }
    return true;
}

Block &Chain::getLastestBlock()
{

    return blockChain.back();
}

void Chain::broadcastLatest()
{
    std::vector<Block> blocks{getLastestBlock()};
    server_instance.broadcast(blocks, LATEST_BLOCK);
}

void Chain::replaceChain(std::vector<Block> newChain)
{

    if (isValidChain() &&
        getAccumulateDifficulty(newChain) > getAccumulateDifficulty(Chain::getInstance()->getBlockChain()))
    {
        blockChain = newChain;
        broadcastLatest();
    }
}

std::vector<Block> Chain::getBlockChain()
{
    return blockChain;
}

bool Chain::addToChain(Block newBlock)
{

    if (isValidNewBlock(newBlock, getLastestBlock()))
    {
        blockChain.push_back(newBlock);
        return true;
    }
    return false;
}

std::string Chain::to_string() const
{

    std::string str;

    for (int i = 0; i < blockChain.size(); ++i)
    {

        str += blockChain.at(i).to_string();
    }
    return str;
}

void handleBlockchainResponse(std::vector<Block> blocks)
{

    // Chain *blockChain = Chain::getInstance()->getBlockChain();
    // Block lastestBlockHeld = blockChain->getLastestBlock();
    // if(latestBlockReceived.index > lastestBlockHeld.index){

    //     if(lastestBlockHeld.hash == latestBlockReceived.prev_hash){
    //         if(blockChain->addToChain(latestBlockReceived)){
    //             blockChain->broadcastLatest();
    //         }
    //     }


    // }
}
