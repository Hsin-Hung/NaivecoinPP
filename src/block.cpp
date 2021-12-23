#include "block.h"
#include "chain.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <openssl/sha.h>

Block Block::genesisBlock(0, "816534932c2b7154836da6afc367695e6337db8a921823784c14378abed4f7d7", NULL, 1465154705, "my genesis block!!");

std::string Block::calculateHash(int index, std::string prev_hash, long timestamp, std::string data)
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
    if(*this == block){
        return true;
    }
    return false;
}

Block::Block(int index, std::string hash, std::string prev_hash, long timestamp, std::string data) : index{index}, hash{hash}, prev_hash{prev_hash}, timestamp{timestamp}, data{data}
{
}

Block Block::generateNextBlock(std::string data)
{

    Block &prev_block = Chain::getInstance()->getLastestBlock();
    int next_index = prev_block.index + 1;
    time_t now = time(nullptr);
    time_t mnow = now * 1000;
    int next_timestamp = mnow;
    std::string next_hash = Block::calculateHash(next_index, prev_block.hash, next_timestamp, data);
    Block new_block = Block(next_index, next_hash, prev_block.hash, next_timestamp, data);
    return new_block;
}
