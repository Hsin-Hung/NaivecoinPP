#ifndef _POW_H_
#define _POW_H_

#include <string>
#include <sstream>
#include <bitset>
#include <iostream>
#include "block.h"
#include "chain.h"
#include <math.h>

#define BLOCK_GENERATION_INTERVAL 10
#define DIFFICULTY_ADJUSTMENT_INTERVAL 10

std::string hexToBin(std::string hexdec);

bool hashMatchesDifficulty(std::string hash, uint64_t difficulty);

Block findBlock(uint64_t index, std::string prev_hash, uint64_t timestamp, std::vector<Transaction> data, uint64_t difficulty);

uint64_t getAdjustedDifficulty(Block latestBLock, std::vector<Block> aBlockChain);

uint64_t getDifficulty(std::vector<Block> aBlockChain);

bool isValidTimestamp(Block newBlock, Block prevBlock);

uint64_t getAccumulateDifficulty(std::vector<Block> blockChain);


#endif