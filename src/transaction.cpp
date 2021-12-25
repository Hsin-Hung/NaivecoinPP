#include "transaction.h"
#include <openssl/sha.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <iomanip>
#include <sstream>
#include <algorithm>

std::vector<UnspentTxOut> unspentTxOuts;

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
    std::string dataToSign = tx.id;
    UnspentTxOut referencedUnspentTxOut = *findUnspentTxOut(txIn.txOutId, txIn.txOutIndex, aUnspentTxOuts);
    std::string referencedAddress = referencedUnspentTxOut.address;
    EC_KEY *eckey = EC_KEY_new();
    BIGNUM const *prv = EC_KEY_get0_private_key(eckey);
    EC_POINT const *pub = EC_KEY_get0_public_key(eckey);
    // ECDSA_SIG *signature = ECDSA_do_sign(privateKey, privateKey.size(), eckey)
    return "public key";
}

UnspentTxOut *findUnspentTxOut(std::string txId, uint64_t index, std::vector<UnspentTxOut> aUnspentTxOuts)
{
    auto iter = std::find_if(aUnspentTxOuts.begin(), aUnspentTxOuts.end(), [&](UnspentTxOut uto)
                          { return uto.txOutId == txId && uto.txOutIndex == index; });
    if (iter == aUnspentTxOuts.end())
        return nullptr;
    return &(*iter);
}

std::vector<UnspentTxOut> newUnspentTxOuts(std::vector<Transaction> newTxs)
{

    std::vector<UnspentTxOut> newUto;
    for (auto newTx : newTxs)
    {
        for (int i = 0; i < newTx.txOuts.size(); ++i)
        {
            TxOut txOut = newTx.txOuts.at(i);
            newUto.push_back(UnspentTxOut(newTx.id, i, txOut.address, txOut.amount));
        }
    }
    return newUto;
}
std::vector<UnspentTxOut> consumedTxOuts(std::vector<Transaction> newTxs)
{
    std::vector<UnspentTxOut> consumedTo;
    for (auto newTx : newTxs)
    {
        for (int i = 0; i < newTx.txIns.size(); ++i)
        {
            TxIn txIn = newTx.txIns.at(i);
            consumedTo.push_back(UnspentTxOut(txIn.txOutId, txIn.txOutIndex, "", 0));
        }
    }
    return consumedTo;
}

std::vector<UnspentTxOut> resultingUnspentTxOuts(std::vector<UnspentTxOut> newUnspentTxOuts, std::vector<UnspentTxOut> consumedTxOuts)
{

    std::vector<UnspentTxOut> resultUto;
    for (auto newUto : newUnspentTxOuts)
    {
        if (findUnspentTxOut(newUto.txOutId, newUto.txOutIndex, consumedTxOuts) == nullptr)
        {

            resultUto.push_back(newUto);
        }
    }

    return resultUto;
}

bool validTxIn(TxIn txIn, Transaction tx, std::vector<UnspentTxOut> aUnspentTxOuts)
{

    // check private and public key with txID

    // check sum of txOutput equal to sum of txInput

    return true;
}
