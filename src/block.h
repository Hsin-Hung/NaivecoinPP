#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <string>

class Block
{

public:
    
    int index;
    std::string hash;
    std::string prev_hash;
    long timestamp;
    std::string data;

    static Block genesisBlock;
    static std::string calculateHash(int index, std::string prev_hash, long timestamp, std::string data);
    static Block generateNextBlock(std::string data);
    Block(int index, std::string hash, std::string prev_hash, long timestamp, std::string data);
    bool operator ==(const Block& d) const;
    bool operator !=(const Block& d) const;
    std::string getBlockHash();
};


#endif