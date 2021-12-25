#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <string>
#include "../include/json.hpp"

class Block
{

public:
    uint64_t index;
    std::string hash;
    std::string prev_hash;
    uint64_t timestamp;
    std::string data;
    uint64_t nonce;
    uint64_t difficulty;
    Block(){};
    Block(uint64_t index, std::string hash, std::string prev_hash, uint64_t timestamp, std::string data, uint64_t nonce, uint64_t difficulty);
    bool operator==(const Block &d) const;
    bool operator!=(const Block &d) const;
    std::string calBlockHash();
    std::string to_string() const;
};

uint64_t getCurrentTimestamp();
void to_json(nlohmann::json &j, const Block &b);
void from_json(const nlohmann::json &j, Block &b);
Block generateNextBlock(std::string data);
std::string calculateHash(uint64_t index, std::string prev_hash, uint64_t timestamp, std::string data, uint64_t nonce, uint64_t difficulty);
bool hasValidHash(Block block);
bool hashMatchesBlockContent(Block block);
extern Block genesisBlock;

#endif