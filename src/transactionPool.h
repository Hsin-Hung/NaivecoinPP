#include "transaction.h"

class TransactionPool
{
public:
    std::vector<Transaction> pool;
    TransactionPool(){};
};

extern TransactionPool transactionPool;