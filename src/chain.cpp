#include "chain.h"
#include "transaction.h"
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

void Chain::broadcastQueryAll()
{

    server_instance.broadcast(std::vector<Block>(), QUERY_ALL);
}

void Chain::replaceChain(std::vector<Block> newChain)
{

    if (isValidChain() &&
        getAccumulateDifficulty(newChain) >= getAccumulateDifficulty(Chain::getInstance()->getBlockChain()))
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
        std::vector<UnspentTxOut> retVal = processTransactions(newBlock.data, unspentTxOuts, newBlock.index);
        blockChain.push_back(newBlock);
        unspentTxOuts = retVal;
        return true;
    }
    return false;
}

void handleBlockchainResponse(std::vector<Block> blocks)
{
    if (blocks.empty())
    {
        std::cout << "Received empty block chain!" << std::endl;
        return;
    }
    Block latestBlockReceived = blocks.back();
    Chain *blockChain = Chain::getInstance();
    Block lastestBlockHeld = blockChain->getLastestBlock();
    if (latestBlockReceived.index > lastestBlockHeld.index)
    {
        std::cout << "blockchain possibly behind. We got "
                  << lastestBlockHeld.index
                  << " . They got "
                  << latestBlockReceived.index << std::endl;

        if (lastestBlockHeld.hash == latestBlockReceived.prev_hash)
        {
            if (blockChain->addToChain(latestBlockReceived))
            {
                blockChain->broadcastLatest();
            }
        }
        else if (blocks.size() == 1)
        {
            std::cout << "We have to query the chain from our peer" << std::endl;
            blockChain->broadcastQueryAll();
        }
        else
        {
            std::cout << "Received blockchain is longer than current blockchain" << std::endl;
            blockChain->replaceChain(blocks);
        }
    }
    else
    {

        std::cout << "Do nothing!" << std::endl;
    }
}
