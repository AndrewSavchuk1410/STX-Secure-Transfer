#pragma once

#include <string>
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace stx {
namespace crypto {

using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

class RSAKey {
public:
    RSAKey(const std::string& path, bool isPrivate);
    EVP_PKEY* get() const noexcept;

private:
    EVP_PKEY_ptr _key;
};

std::vector<uint8_t> generateSessionKey(size_t keySize = 32);

std::vector<uint8_t> rsaEncrypt(EVP_PKEY* key, const std::vector<uint8_t>& data);
std::vector<uint8_t> rsaDecrypt(EVP_PKEY* key, const std::vector<uint8_t>& enc);

std::vector<uint8_t> aesEncrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key, std::vector<uint8_t>& iv_out);
std::vector<uint8_t> aesDecrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

}  
}
