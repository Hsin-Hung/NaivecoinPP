#include <string>

class Block
{
    public:
        int index;
        std::string hash;
        std::string prev_hash;
        int timestamp;
        std::string data;
};