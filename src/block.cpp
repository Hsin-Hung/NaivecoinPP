#include "block.h"
#include "chain.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <openssl/sha.h>

Block::Block(uint64_t index, std::string hash, std::string prev_hash, uint64_t timestamp, std::string data) : index{index}, hash{hash}, prev_hash{prev_hash}, timestamp{timestamp}, data{data}
{
}

void to_json(nlohmann::json &j, const Block &b)
{
    j["index"] = b.index;
    j["hash"] = b.hash;
    j["prev_hash"] = b.prev_hash;
    j["timestamp"] = b.timestamp;
    j["data"] = b.data;
}

void from_json(const nlohmann::json &j, Block &b)
{

    j.at("index").get_to(b.index);
    j.at("hash").get_to(b.hash);
    j.at("prev_hash").get_to(b.prev_hash);
    j.at("timestamp").get_to(b.timestamp);
    j.at("data").get_to(b.data);
}

std::string calculateHash(uint64_t index, std::string prev_hash, uint64_t timestamp, std::string data)
{
    unsigned char sha_hash[SHA256_DIGEST_LENGTH];
    std::string str = std::to_string(index) + prev_hash + std::to_string(timestamp) + data;
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

std::string Block::getBlockHash()
{

    return calculateHash(index, prev_hash, timestamp, data);
}
bool Block::operator==(const Block &block) const
{

    return index == block.index && hash == block.hash && prev_hash == block.prev_hash && timestamp == block.timestamp && data == block.data;
}

bool Block::operator!=(const Block &block) const
{
    if (*this == block)
    {
        return true;
    }
    return false;
}

Block generateNextBlock(std::string data)
{
    Chain *chain = Chain::getInstance();
    Block &prev_block = chain->getLastestBlock();
    uint64_t next_index = prev_block.index + 1;
    time_t now = time(nullptr);
    time_t mnow = now * 1000;
    uint64_t next_timestamp = mnow;
    std::string next_hash = calculateHash(next_index, prev_block.hash, next_timestamp, data);
    Block newBlock = Block(next_index, next_hash, prev_block.hash, next_timestamp, data);
    chain->addToChain(newBlock);
    chain->broadcastLatest();
    return newBlock;
}

std::string Block::to_string() const
{

    std::string str = "index : " + std::to_string(index) + " / ";
    str += "hash : " + hash + " / ";
    str += "prev_hash : " + prev_hash + " / ";
    str += "timestamp : " + std::to_string(timestamp) + " / ";
    str += "data : " + data + "\n";
    return str;
}

Block genesisBlock(0, "816534932c2b7154836da6afc367695e6337db8a921823784c14378abed4f7d7", "", 1465154705, "my genesis block!!");