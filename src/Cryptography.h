/**
 * Copyright (C) 2019-2022 polistern
 *
 * This file is part of pboted and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#ifndef PBOTE_SRC_CRYPTOGRAPHY_H_
#define PBOTE_SRC_CRYPTOGRAPHY_H_

#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <openssl/conf.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "Logging.h"

namespace pbote
{

/// AES
#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 32
#define EPH_KEY_LEN 33
/// ECDHP256
#define ECDHP256_PUB_KEY 33
#define ECDHP256_PRIV_KEY 33
/// ECDHP521
#define ECDHP521_PUB_KEY 66
#define ECDHP521_PRIV_KEY 67
#define ECDHP521_PRIV_KEY_COMPRESSED 66
  
typedef uint8_t byte;

using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

class CryptoKeyEncryptor
{
 public:
  virtual ~CryptoKeyEncryptor () {};
  virtual std::vector<byte> Encrypt (const uint8_t *data, int dlen) = 0;
};

class CryptoKeyDecryptor
{
 public:
  virtual ~CryptoKeyDecryptor () {};
  virtual std::vector<byte> Decrypt (const uint8_t *encrypted, int elen) = 0;
  virtual size_t GetPublicKeyLen () const = 0;
};

class ECDHP256Encryptor : public CryptoKeyEncryptor
{
 public:
  ECDHP256Encryptor (const byte *pubkey);
  ~ECDHP256Encryptor () override;
  std::vector<byte> Encrypt (const byte *data, int dlen) override;

 private:
  EC_GROUP *ec_curve;
  EC_POINT *ec_public_point;
  EC_KEY *ec_ephemeral_key;

  std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> rbe;
};

class ECDHP256Decryptor : public CryptoKeyDecryptor
{
 public:
  ECDHP256Decryptor (const byte *priv);
  ~ECDHP256Decryptor () override;
  std::vector<byte> Decrypt (const byte *encrypted, int elen) override;
  size_t GetPublicKeyLen () const override { return 33; };

 private:
  EC_GROUP *ec_curve;
  BIGNUM *bn_private_key;
};

class ECDHP521Encryptor : public CryptoKeyEncryptor
{
 public:
  ECDHP521Encryptor (const byte *pubkey);
  ~ECDHP521Encryptor () override;
  std::vector<byte> Encrypt (const byte *data, int dlen) override;

 private:
  EC_GROUP *ec_curve;
  EC_POINT *ec_public_point;
  EC_KEY *ec_ephemeral_key;

  std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t> rbe;
};

class ECDHP521Decryptor : public CryptoKeyDecryptor
{
 public:
  ECDHP521Decryptor (const byte *priv);
  ~ECDHP521Decryptor () override;
  std::vector<byte> Decrypt (const byte *encrypted, int elen) override;
  size_t GetPublicKeyLen () const override { return ECDHP521_PUB_KEY; };

 private:
  EC_GROUP *ec_curve;
  BIGNUM *bn_private_key;
};

inline byte *
get_secret (EC_KEY *private_key, const EC_POINT *public_key, int *secret_len)
{
  int field_size;
  unsigned char *secret;

  field_size = EC_GROUP_get_degree (EC_KEY_get0_group (private_key));
  *secret_len = (field_size + 7) / 8;

  if (nullptr == (secret = static_cast<byte *> (OPENSSL_malloc (*secret_len))))
    {
      LogPrint(eLogError, "Crypto: get_secret: Failed to allocate memory for secret");
      return nullptr;
    }

  *secret_len = ECDH_compute_key (secret, *secret_len, public_key, private_key, nullptr);

  if (*secret_len < 0)
  {
    OPENSSL_free (secret);
    LogPrint (eLogError, "Crypto: get_secret: Failed to compute agreement key");
    return nullptr;
  }

  if (*secret_len == 0)
    {
      OPENSSL_free (secret);
      LogPrint (eLogError, "Crypto: get_secret: Secret have zero length");
      return nullptr;
    }

  return secret;
}

inline EC_KEY *
create_key ()
{
  EC_KEY *key;

  if (nullptr == (key = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1)))
    {
      LogPrint(eLogError, "Crypto: create_key: Failed to create key curve");
      return nullptr;
    }

  if (1 != EC_KEY_generate_key (key))
    {
      LogPrint (eLogError, "Crypto: create_key: Failed to generate key");
      return nullptr;
    }

  return key;
}

inline bool
bn2buf (const BIGNUM *bn, uint8_t *buf, size_t len)
{
  int offset = len - BN_num_bytes (bn);

  if (offset < 0)
    return false;

  BN_bn2bin (bn, buf + offset);
  memset (buf, 0, offset);

  return true;
}

void aes_encrypt (const byte key[AES_KEY_SIZE], const byte iv[AES_BLOCK_SIZE],
                  const std::vector<byte>& pdata, std::vector<byte>& cdata);
void aes_decrypt (const byte key[AES_KEY_SIZE], const byte iv[AES_BLOCK_SIZE],
                  const std::vector<byte>& cdata, std::vector<byte>& pdata);

} // namespace pbote

#endif //PBOTE_SRC_CRYPTOGRAPHY_H_
