#include "chain.h"

Chain *Chain::instance = nullptr;

Chain::Chain() : blockChain{Block::genesisBlock} {};

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

    if (Block::genesisBlock != blockChain.at(0))
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

void Chain::replaceChain(std::vector<Block> newChain)
{

    if (isValidChain() && newChain.size() > blockChain.size())
    {
        blockChain = newChain;
        broadcastLatest();
    }
}

void Chain::broadcastLatest() {}
