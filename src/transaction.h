#ifndef _TX_H_
#define _TX_H_

#include "../include/json.hpp"
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/opensslv.h>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <utility>
#include <string>
#include <vector>

#define COINBASE_AMOUNT 50

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
typedef struct ECDSA_SIG_st {
    BIGNUM *r;
    BIGNUM *s;
} ECDSA_SIG;
#endif
class TxOut
{
public:
    std::string address;
    uint64_t amount;
    TxOut(){};
    TxOut(std::string address, uint64_t amount) : address{address}, amount{amount} {}
    bool operator==(const TxOut &txOut) const;
    bool operator!=(const TxOut &txOut) const;
};

class TxIn
{
public:
    std::string txOutId;
    uint64_t txOutIndex;
    std::string signature;
    TxIn(){};
    TxIn(std::string txOutId, uint64_t txOutIndex, std::string signature) : txOutId{txOutId}, txOutIndex{txOutIndex}, signature{signature} {}
    bool operator==(const TxIn &txIn) const;
    bool operator!=(const TxIn &txIn) const;
};

class Transaction
{
public:
    std::string id;
    std::vector<TxIn> txIns;
    std::vector<TxOut> txOuts;
    bool operator==(const Transaction &b) const;
    bool operator!=(const Transaction &b) const;
};

class UnspentTxOut
{
public:
    std::string txOutId;
    uint64_t txOutIndex;
    std::string address;
    uint64_t amount;
    UnspentTxOut() = default;
    UnspentTxOut(std::string txOutId, uint64_t txOutIndex, std::string address, uint64_t amount) : txOutId{txOutId}, txOutIndex{txOutIndex}, address{address}, amount{amount} {}
};

extern std::vector<UnspentTxOut> unspentTxOuts;
extern EC_KEY *eckey;
std::vector<UnspentTxOut> updateUnspentTxOuts(std::vector<Transaction> newTransactions, std::vector<UnspentTxOut> aUnspentTxOuts);
std::string getTransactionId(Transaction tx);
std::string signTxIn(Transaction tx, uint64_t txInIndex, std::string privateKey, std::vector<UnspentTxOut> aUnspentTxOuts);
UnspentTxOut *findUnspentTxOut(std::string txId, uint64_t index, std::vector<UnspentTxOut> aUnspentTxOuts);
bool validTxIn(TxIn txIn, Transaction tx, std::vector<UnspentTxOut> aUnspentTxOuts);
bool validateTransaction(Transaction tx, std::vector<UnspentTxOut> aUnspentTxOuts);
void generate_key_pair(std::string &pub_key, std::string &priv_key);
void to_json(nlohmann::json &j, const Transaction &tx);
void from_json(const nlohmann::json &j, Transaction &tx);
void to_json(nlohmann::json &j, const TxOut &txOut);
void from_json(const nlohmann::json &j, TxOut &txOut);
void to_json(nlohmann::json &j, const TxIn &txIn);
void from_json(const nlohmann::json &j, TxIn &txIn);
std::string signTxIn(Transaction tx, uint64_t txInIndex, std::string privateKey, std::vector<UnspentTxOut> unspentTxOuts);
Transaction getCoinbaseTransaction(std::string address, uint64_t blockIndex);
std::vector<UnspentTxOut> processTransactions(std::vector<Transaction> aTransactions, std::vector<UnspentTxOut> aUnspentTxOuts, uint64_t blockIndex);
std::pair<std::string, std::string> getSignatureRS(std::string signature, std::string delimiter);
#endif