#ifndef _WALLET_H_
#define _WALLET_H_
#include <string>
#include "transaction.h"

#define PRIVATE_KEY_LOCATION "private_key"

class Wallet
{
public:
    Wallet();
    std::string generatePrivateKey();
    std::string getPublicKey();
    std::string getPrivateKey();
    uint64_t getAccountBalance();
    uint64_t getBalance(std::string address, std::vector<UnspentTxOut> unspentTxOuts);
    std::pair<std::vector<UnspentTxOut>, uint64_t> findTxOutsForAmount(u_int64_t amount, std::vector<UnspentTxOut> myUnspentTxOuts);
    TxIn toUnsignedTxIn(UnspentTxOut unspentTxOut);
    Transaction createTransaction(std::string receiverAddress, uint64_t amount, std::string privateKey, std::vector<UnspentTxOut> unspentTxOuts);
    std::vector<TxOut> createTxOuts(std::string receiverAddress, std::string myAddress, uint64_t amount, uint64_t leftOverAmount);
};

extern Wallet wallet;

#endif