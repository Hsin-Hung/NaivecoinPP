#include "wallet.h"
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

inline bool file_exists(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

Wallet wallet;

Wallet::Wallet()
{
    generatePrivateKey();
}

uint64_t Wallet::getAccountBalance()
{

    return getBalance(getPublicKey(), unspentTxOuts);
}

std::string Wallet::generatePrivateKey()
{
    if (!file_exists(PRIVATE_KEY_LOCATION))
    {
        generatePrivateKey();
    }
    std::cout << "generate private key" << std::endl;
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    point_conversion_form_t form = EC_GROUP_get_point_conversion_form(group);

    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

    if (ec_key == NULL)
    {
        std::cerr << "Unable to allocate key structure\n";
        exit(1);
    }
    /* Generate a random key. */
    if (EC_KEY_generate_key(ec_key) != 1)
    {
        std::cerr << "Unable to generate keypair\n";
        exit(1);
    }
    /* Verify that the keypair is OK. */
    if (!EC_KEY_check_key(ec_key))
    {
        std::cerr << "Invalid key pair\n";
        return "";
    }

    const EC_POINT *pubkey = EC_KEY_get0_public_key(ec_key);
    {
        BN_CTX *ctx;

        ctx = BN_CTX_new();
        std::string pubkey_str = EC_POINT_point2hex(group, pubkey, form, ctx);
        std::cout << "--pubkey " << pubkey_str << "\n";
        BN_CTX_free(ctx);
    }

    const BIGNUM *privkey = EC_KEY_get0_private_key(ec_key);
    std::cout << "--privkey " << BN_bn2hex(privkey) << "\n";

    BIO *bio = BIO_new_fp(stdout, 0);
    // assert(1==EC_KEY_print(bio, ec_key, 0));
    BIO_free(bio);
    {
        FILE *f = fopen(PRIVATE_KEY_LOCATION, "w");
        PEM_write_ECPrivateKey(f, ec_key, NULL, NULL, 0, NULL, NULL);
        // PEM_write_bio_ECPrivateKey(bio,ec_key, NULL,NULL,0,NULL,NULL);
        fclose(f);
    }

    std::string privateKeyStr = BN_bn2hex(privkey);

    EC_KEY_free(ec_key);
    EC_GROUP_free(group);
    return privateKeyStr;
}

std::string Wallet::getPrivateKey()
{

    std::cout << "get private key" << std::endl;
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    {
        FILE *f = fopen(PRIVATE_KEY_LOCATION, "r");
        PEM_read_ECPrivateKey(f, &ec_key, NULL, NULL);
        fclose(f);
    }
    auto privateKey = EC_KEY_get0_private_key(ec_key);
    std::cout << "--privkey " << BN_bn2hex(privateKey) << "\n";
    std::string privateKeyStr = BN_bn2hex(privateKey);
    EC_KEY_free(ec_key);

    return privateKeyStr;
}

std::string Wallet::getPublicKey()
{
    std::cout << "get public key" << std::endl;
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    point_conversion_form_t form = EC_GROUP_get_point_conversion_form(group);
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

    {
        FILE *f = fopen(PRIVATE_KEY_LOCATION, "r");
        PEM_read_ECPrivateKey(f, &ec_key, NULL, NULL);
        fclose(f);
    }
    std::string pubkey_str;
    const EC_POINT *pubkey = EC_KEY_get0_public_key(ec_key);
    {
        BN_CTX *ctx;

        ctx = BN_CTX_new();
        pubkey_str = EC_POINT_point2hex(group, pubkey, form, ctx);
        std::cout << "--pubkey " << pubkey_str << "\n";
        BN_CTX_free(ctx);
    }
    return pubkey_str;
}

uint64_t Wallet::getBalance(std::string address, std::vector<UnspentTxOut> unspentTxOuts)
{

    uint64_t balance{0};

    for (auto uTxo : unspentTxOuts)
    {

        if (uTxo.address == address)
        {
            balance += uTxo.amount;
        }
    }
    return balance;
}

std::pair<std::vector<UnspentTxOut>, uint64_t> Wallet::findTxOutsForAmount(u_int64_t amount, std::vector<UnspentTxOut> myUnspentTxOuts)
{

    uint64_t currentAmount{0};
    std::vector<UnspentTxOut> includedUnspentTxOuts;

    for (auto myUto : myUnspentTxOuts)
    {
        includedUnspentTxOuts.push_back(myUto);
        currentAmount += myUto.amount;
        if (currentAmount > amount)
        {
            return std::make_pair(includedUnspentTxOuts, currentAmount - amount);
        }
    }

    return std::make_pair(includedUnspentTxOuts, 0);
}

TxIn Wallet::toUnsignedTxIn(UnspentTxOut unspentTxOut)
{

    TxIn txIn;
    txIn.txOutId = unspentTxOut.txOutId;
    txIn.txOutIndex = unspentTxOut.txOutIndex;

    return txIn;
}

std::vector<TxOut> Wallet::createTxOuts(std::string receiverAddress, std::string myAddress, uint64_t amount, uint64_t leftOverAmount)
{
    std::vector<TxOut> txOuts;
    TxOut txOut1(receiverAddress, amount), leftOverTx(myAddress, leftOverAmount);
    if (leftOverAmount == 0)
    {
        txOuts.push_back(txOut1);
    }
    else
    {
        txOuts.push_back(leftOverTx);
    }

    return txOuts;
}

Transaction Wallet::createTransaction(std::string receiverAddress, uint64_t amount, std::string privateKey, std::vector<UnspentTxOut> unspentTxOuts)
{
    std::cout << "createTransaction" << std::endl;
    std::string myAddress = getPublicKey();
    std::vector<UnspentTxOut> myUnspentTxOuts;

    for (auto uTo : unspentTxOuts)
    {
        if (uTo.address == myAddress)
        {

            myUnspentTxOuts.push_back(uTo);
        }
    }

    std::pair<std::vector<UnspentTxOut>, uint64_t> txOutsForAmount = findTxOutsForAmount(amount, myUnspentTxOuts);
    std::vector<TxIn> unsignedTxIns;
    for (auto uTo : txOutsForAmount.first)
    {

        unsignedTxIns.push_back(toUnsignedTxIn(uTo));
    }

    Transaction tx;
    tx.txIns = unsignedTxIns;
    tx.txOuts = createTxOuts(receiverAddress, myAddress, amount, txOutsForAmount.second);
    tx.id = getTransactionId(tx);

    for (int i = 0; i < tx.txIns.size(); ++i)
    {

        tx.txIns[i].signature = signTxIn(tx, i, privateKey, unspentTxOuts);
    }

    return tx;
}
