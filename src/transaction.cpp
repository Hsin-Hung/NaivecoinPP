#include "transaction.h"
#include "wallet.h"
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>

EC_KEY *eckey = EC_KEY_new();
std::vector<UnspentTxOut> unspentTxOuts;

bool TxOut::operator==(const TxOut &txOut) const
{
    return address == txOut.address && amount == txOut.amount;
}

bool TxOut::operator!=(const TxOut &txOut) const
{
    return !(*this == txOut);
}

bool TxIn::operator==(const TxIn &txIn) const
{
    return txOutId == txIn.txOutId && txOutIndex == txIn.txOutIndex && signature == txIn.signature;
}

bool TxIn::operator!=(const TxIn &txIn) const
{
    return !(*this == txIn);
}

bool Transaction::operator==(const Transaction &b) const
{

    return id == b.id && txIns == b.txIns && txOuts == b.txOuts;
}
bool Transaction::operator!=(const Transaction &b) const
{
    return !(*this == b);
}

std::string getTransactionId(Transaction tx)
{

    std::string str;
    for (auto txIn : tx.txIns)
    {
        str += txIn.txOutId + std::to_string(txIn.txOutIndex);
    }
    for (auto txOut : tx.txOuts)
    {
        str += txOut.address + std::to_string(txOut.amount);
    }
    unsigned char sha_hash[SHA256_DIGEST_LENGTH];
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

std::string signTxIn(Transaction tx, uint64_t txInIndex, std::string privateKey, std::vector<UnspentTxOut> aUnspentTxOuts)
{

    TxIn txIn = tx.txIns.at(txInIndex);
    const unsigned char *dataToSign = (unsigned char *)tx.id.c_str();
    UnspentTxOut referencedUnspentTxOut = *findUnspentTxOut(txIn.txOutId, txIn.txOutIndex, aUnspentTxOuts);
    std::string referencedAddress = referencedUnspentTxOut.address;

    if (wallet.getPublicKey() != referencedAddress)
    {
        std::cout << "Public key doesnt match referenced address!" << std::endl;
        return "";
    }

    ECDSA_SIG *sig;
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    {
        FILE *f = fopen(PRIVATE_KEY_LOCATION, "r");
        PEM_read_ECPrivateKey(f, &ec_key, NULL, NULL);
        fclose(f);
    }
    auto pKey = EC_KEY_get0_private_key(ec_key);

    sig = ECDSA_do_sign_ex(dataToSign, 32, NULL, NULL, ec_key);
    if (sig == NULL)
    {
        std::cerr << "Unable to sign digest\n";
        return "";
    }
    int verified;

    verified = ECDSA_do_verify(dataToSign,
                               32,
                               sig, ec_key);
    if (verified != 1)
    {
        std::cerr << "Unable to verify signature\n";
        return "";
    }

    ECDSA_SIG_free(sig);
    EC_KEY_free(ec_key);
    return "";
}

UnspentTxOut *findUnspentTxOut(std::string txId, uint64_t index, std::vector<UnspentTxOut> aUnspentTxOuts)
{
    auto iter = std::find_if(aUnspentTxOuts.begin(), aUnspentTxOuts.end(), [&](UnspentTxOut uto)
                             { return uto.txOutId == txId && uto.txOutIndex == index; });
    if (iter == aUnspentTxOuts.end())
        return nullptr;
    return &(*iter);
}

std::vector<UnspentTxOut> processTransactions(std::vector<Transaction> aTransactions, std::vector<UnspentTxOut> aUnspentTxOuts, uint64_t blockIndex)
{

    return updateUnspentTxOuts(aTransactions, aUnspentTxOuts);
}

std::vector<UnspentTxOut> updateUnspentTxOuts(std::vector<Transaction> newTransactions, std::vector<UnspentTxOut> aUnspentTxOuts)
{

    std::vector<UnspentTxOut> newUnspentTxOuts;
    for (auto &newTx : newTransactions)
    {
        for (int i = 0; i < newTx.txOuts.size(); ++i)
        {
            TxOut txOut = newTx.txOuts.at(i);
            newUnspentTxOuts.push_back(UnspentTxOut(newTx.id, i, txOut.address, txOut.amount));
        }
    }

    std::vector<UnspentTxOut> consumedTxOuts;
    for (auto &newTx : newTransactions)
    {
        for (int i = 0; i < newTx.txIns.size(); ++i)
        {
            TxIn txIn = newTx.txIns.at(i);
            consumedTxOuts.push_back(UnspentTxOut(txIn.txOutId, txIn.txOutIndex, "", 0));
        }
    }

    std::vector<UnspentTxOut> resultUto;
    for (auto &utxo : aUnspentTxOuts)
    {
        if (findUnspentTxOut(utxo.txOutId, utxo.txOutIndex, consumedTxOuts) == nullptr)
        {

            resultUto.push_back(utxo);
        }
    }

    resultUto.insert(resultUto.end(), newUnspentTxOuts.begin(), newUnspentTxOuts.end());
    return resultUto;
}

bool validateTransaction(Transaction tx, std::vector<UnspentTxOut> aUnspentTxOuts)
{

    if (getTransactionId(tx) != tx.id)
    {
        std::cout << "Transaction ID doesn't match!" << std::endl;
        return false;
    }

    uint64_t totalTxIn{0};
    for (auto txIn : tx.txIns)
    {

        totalTxIn += findUnspentTxOut(txIn.txOutId, txIn.txOutIndex, aUnspentTxOuts)->amount;
    }

    for (auto txOut : tx.txOuts)
    {

        totalTxIn -= txOut.amount;
    }

    if (totalTxIn)
    {
        std::cout << "Transaction Output and Input amount don't match!" << std::endl;
        return false;
    }

    return true;
}
bool validTxIn(TxIn txIn, Transaction tx, std::vector<UnspentTxOut> aUnspentTxOuts)
{
    // UnspentTxOut referencedUTxOut;
    // for (auto &uTo : aUnspentTxOuts)
    // {
    //     if (uTo.txOutId == txIn.txOutId && uTo.txOutIndex == txIn.txOutIndex)
    //     {
    //         referencedUTxOut = uTo;
    //     }
    // }
    // std::string address = referencedUTxOut.address;

    return true;
}

void generate_key_pair(std::string &pub_key, std::string &priv_key)
{
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    assert(1 == EC_KEY_generate_key(ec_key));
    assert(1 == EC_KEY_check_key(ec_key));

    BIO *bio = BIO_new_fp(stdout, 0);
    // assert(1==EC_KEY_print(bio, ec_key, 0));
    BIO_free(bio);

    {
        FILE *f = fopen(pub_key.c_str(), "w");
        PEM_write_EC_PUBKEY(f, ec_key);
        // PEM_write_bio_EC_PUBKEY(bio, ec_key);
        fclose(f);
    }

    {
        FILE *f = fopen(priv_key.c_str(), "w");
        PEM_write_ECPrivateKey(f, ec_key, NULL, NULL, 0, NULL, NULL);
        // PEM_write_bio_ECPrivateKey(bio,ec_key, NULL,NULL,0,NULL,NULL);
        fclose(f);
    }

    EC_KEY_free(ec_key);
}

Transaction getCoinbaseTransaction(std::string address, uint64_t blockIndex)
{

    Transaction t;
    TxIn txIn;
    txIn.signature = "";
    txIn.txOutId = "";
    txIn.txOutIndex = blockIndex;

    t.txIns = std::vector<TxIn>{txIn};
    t.txOuts = std::vector<TxOut>{TxOut(address, COINBASE_AMOUNT)};
    t.id = getTransactionId(t);
    return t;
}

void to_json(nlohmann::json &j, const Transaction &t)
{
    j["id"] = t.id;
    j["txIns"] = t.txIns;
    j["txOuts"] = t.txOuts;
}

void from_json(const nlohmann::json &j, Transaction &t)
{
    j.at("id").get_to(t.id);
    j.at("txIns").get_to<std::vector<TxIn>>(t.txIns);
    j.at("txOuts").get_to<std::vector<TxOut>>(t.txOuts);
}

void to_json(nlohmann::json &j, const TxOut &txOut)
{
    j["address"] = txOut.address;
    j["amount"] = txOut.amount;
}
void from_json(const nlohmann::json &j, TxOut &txOut)
{
    j.at("address").get_to(txOut.address);
    j.at("amount").get_to(txOut.amount);
}
void to_json(nlohmann::json &j, const TxIn &txIn)
{
    j["txOutId"] = txIn.txOutId;
    j["txOutIndex"] = txIn.txOutIndex;
    j["signature"] = txIn.signature;
}
void from_json(const nlohmann::json &j, TxIn &txIn)
{
    j.at("txOutId").get_to(txIn.txOutId);
    j.at("txOutIndex").get_to(txIn.txOutIndex);
    j.at("signature").get_to(txIn.signature);
}