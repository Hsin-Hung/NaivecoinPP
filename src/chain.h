#include <vector>
#include "block.h"

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
    bool isValidBlockStructure(Block &block);
    bool isValidChain();
    void broadcastLatest();
};