#include "chain.h"

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
    else if (newBlock.getBlockHash() != newBlock.hash)
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
        if (isValidNewBlock(blockChain.at(i - 1), blockChain.at(i)))
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

void Chain::broadcastLatest() {}

void Chain::replaceChain(std::vector<Block> newChain)
{

    if (isValidChain() && newChain.size() > blockChain.size())
    {
        blockChain = newChain;
        broadcastLatest();
    }
}

std::vector<Block> Chain::getBlockChain()
{
    return blockChain;
}

void Chain::addToChain(Block newBlock)
{

    if (isValidNewBlock(newBlock, getLastestBlock()))
    {
        blockChain.push_back(newBlock);
    }
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

