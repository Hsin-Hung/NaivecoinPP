#include "block.h"
#include "chain.h"
#include "poW.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <openssl/sha.h>

Block genesisBlock(0, "816534932c2b7154836da6afc367695e6337db8a921823784c14378abed4f7d7", "", 1465154705, "my genesis block!!", 0, 0);

std::string calculateHash(uint64_t index, std::string prev_hash, uint64_t timestamp, std::string data, uint64_t nonce, uint64_t difficulty)
{
    unsigned char sha_hash[SHA256_DIGEST_LENGTH];
    std::string str = std::to_string(index) + prev_hash + std::to_string(timestamp) + data + std::to_string(nonce) + std::to_string(difficulty);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(sha_hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)sha_hash[i];
    }
    return ss.str();
}

std::string Block::calBlockHash()
{

    return calculateHash(index, prev_hash, timestamp, data, nonce, difficulty);
}
bool Block::operator==(const Block &block) const
{

    return index == block.index && hash == block.hash && prev_hash == block.prev_hash && timestamp == block.timestamp && data == block.data;
}

bool Block::operator!=(const Block &block) const
{
    return *this == block;
}

uint64_t getCurrentTimestamp()
{

    time_t now = time(nullptr);
    time_t mnow = now * 1000;
    uint64_t cur_timestamp = mnow;
    return cur_timestamp;
}

Block generateNextBlock(std::string data)
{
    Chain *chain = Chain::getInstance();
    Block &prev_block = chain->getLastestBlock();
    uint64_t difficulty = getDifficulty(Chain::getInstance()->getBlockChain());
    uint64_t next_index = prev_block.index + 1;
    uint64_t next_timestamp = getCurrentTimestamp();
    Block newBlock = findBlock(next_index, prev_block.hash, next_timestamp, data, difficulty);
    chain->addToChain(newBlock);
    chain->broadcastLatest();
    return newBlock;
}

bool hasValidHash(Block block)
{
    if (!hashMatchesBlockContent(block) || !hashMatchesDifficulty(block.hash, block.difficulty))
    {
        return false;
    }

    return true;
}
bool hashMatchesBlockContent(Block block)
{

    std::string hash = block.calBlockHash();
    return hash == block.hash;
}

void to_json(nlohmann::json &j, const Block &b)
{
    j["index"] = b.index;
    j["hash"] = b.hash;
    j["prev_hash"] = b.prev_hash;
    j["timestamp"] = b.timestamp;
    j["data"] = b.data;
    j["nonce"] = b.nonce;
    j["difficulty"] = b.difficulty;
}

void from_json(const nlohmann::json &j, Block &b)
{

    j.at("index").get_to(b.index);
    j.at("hash").get_to(b.hash);
    j.at("prev_hash").get_to(b.prev_hash);
    j.at("timestamp").get_to(b.timestamp);
    j.at("data").get_to(b.data);
    j.at("nonce").get_to(b.nonce);
    j.at("difficulty").get_to(b.difficulty);
}