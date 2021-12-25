#include <string>
#include <vector>

#define COINBASE_AMOUNT 50

class TxOut
{
public:
    std::string address;
    uint64_t amount;

    TxOut(std::string address, uint64_t amount) : address{address}, amount{amount} {}
};

class TxIn
{
public:
    std::string txOutId;
    uint64_t txOutIndex;
    std::string signature;

    TxIn(std::string txOutId, uint64_t txOutIndex, std::string signature) : txOutId{txOutId}, txOutIndex{txOutIndex}, signature{signature} {}
};

class Transaction
{
public:
    std::string id;
    std::vector<TxIn> txIns;
    std::vector<TxOut> txOuts;
};

class UnspentTxOut
{
public:
    const std::string txOutId;
    const uint64_t txOutIndex;
    const std::string address;
    const uint64_t amount;
    UnspentTxOut(std::string txOutId, uint64_t txOutIndex, std::string address, uint64_t amount) : txOutId{txOutId}, txOutIndex{txOutIndex}, address{address}, amount{amount} {}
};

extern std::vector<UnspentTxOut> unspentTxOuts;

std::vector<UnspentTxOut> newUnspentTxOuts(std::vector<Transaction> newTxs);
std::vector<UnspentTxOut> consumedTxOuts(std::vector<Transaction> newTxs);
std::vector<UnspentTxOut> resultingUnspentTxOuts(std::vector<UnspentTxOut>  newUnspentTxOuts, std::vector<UnspentTxOut> consumedTxOuts);
std::string getTransactionId(Transaction tx);
std::string signTxIn(Transaction tx, uint64_t txInIndex, std::string privateKey, std::vector<UnspentTxOut> aUnspentTxOuts);
UnspentTxOut *findUnspentTxOut(std::string txId, uint64_t index, std::vector<UnspentTxOut> aUnspentTxOuts);
bool validTxIn(TxIn txIn, Transaction tx, std::vector<UnspentTxOut> aUnspentTxOuts);