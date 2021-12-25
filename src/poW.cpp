#include "poW.h"

std::string hexToBin(std::string hexdec)
{
    std::string binStr;

    long int i = 0;

    while (hexdec[i])
    {

        switch (hexdec[i])
        {
        case '0':
            binStr += "0000";
            break;
        case '1':
            binStr += "0001";
            break;
        case '2':
            binStr += "0010";
            break;
        case '3':
            binStr += "0011";
            break;
        case '4':
            binStr += "0100";
            break;
        case '5':
            binStr += "0101";
            break;
        case '6':
            binStr += "0110";
            break;
        case '7':
            binStr += "0111";
            break;
        case '8':
            binStr += "1000";
            break;
        case '9':
            binStr += "1001";
            break;
        case 'A':
        case 'a':
            binStr += "1010";
            break;
        case 'B':
        case 'b':
            binStr += "1011";
            break;
        case 'C':
        case 'c':
            binStr += "1100";
            break;
        case 'D':
        case 'd':
            binStr += "1101";
            break;
        case 'E':
        case 'e':
            binStr += "1110";
            break;
        case 'F':
        case 'f':
            binStr += "1111";
            break;
        default:
            std::cout << "\nInvalid hexadecimal digit "
                      << hexdec[i];
        }
        i++;
    }

    return binStr;
}
bool hashMatchesDifficulty(std::string hash, uint64_t difficulty)
{
    std::string binHash = hexToBin(hash);
    for (auto &c : binHash)
    {
        if (c == '0')
        {
            --difficulty;
        }
        else if (difficulty != 0)
        {
            return false;
        }

        if (!difficulty)
            return true;
    }

    return false;
}

Block findBlock(uint64_t index, std::string prev_hash, uint64_t timestamp, std::string data, uint64_t difficulty)
{
    uint64_t nonce = 0;

    while (1)
    {
        std::string hash = calculateHash(index, prev_hash, timestamp, data, nonce, difficulty);
        if (hashMatchesDifficulty(hash, difficulty))
        {
            return Block(index, hash, prev_hash, timestamp, data, nonce, difficulty);
        }

        ++nonce;
    }
}

uint64_t getAdjustedDifficulty(Block latestBLock, std::vector<Block> aBlockChain)
{
    Block prevAdjustmentBlock = aBlockChain[Chain::getInstance()->getBlockChain().size() - DIFFICULTY_ADJUSTMENT_INTERVAL];
    uint64_t timeExpected = BLOCK_GENERATION_INTERVAL * DIFFICULTY_ADJUSTMENT_INTERVAL;
    uint64_t timeTaken = latestBLock.timestamp - prevAdjustmentBlock.timestamp;
    if (timeTaken < timeExpected / 2)
    {
        return prevAdjustmentBlock.difficulty + 1;
    }
    else if (timeTaken > timeExpected * 2)
    {
        return prevAdjustmentBlock.difficulty - 1;
    }
    else
    {
        return prevAdjustmentBlock.difficulty;
    }
}

uint64_t getDifficulty(std::vector<Block> aBlockChain)
{
    Block latestBlock = aBlockChain[Chain::getInstance()->getBlockChain().size() - 1];
    if (latestBlock.index % DIFFICULTY_ADJUSTMENT_INTERVAL == 0 && latestBlock.index != 0)
    {
        return getAdjustedDifficulty(latestBlock, aBlockChain);
    }
    else
    {
        return latestBlock.difficulty;
    }
}

bool isValidTimestamp(Block newBlock, Block prevBlock)
{

    return (prevBlock.timestamp - 60000 < newBlock.timestamp) && (newBlock.timestamp - 60000 < getCurrentTimestamp());
}

uint64_t getAccumulateDifficulty(std::vector<Block> blockChain)
{

    uint64_t sum{0};
    for (auto &b : blockChain)
    {
        sum += pow(2, b.difficulty);
    }

    return sum;
}