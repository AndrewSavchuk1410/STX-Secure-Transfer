#include "crypto.h"
#include <openssl/rand.h>
#include <openssl/err.h>
#include <fstream>
#include <vector>

namespace stx {
namespace crypto {

RSAKey::RSAKey(const std::string& path, bool isPrivate)
    : _key(nullptr, EVP_PKEY_free) {
    FILE* fp = nullptr;
    fopen_s(&fp, path.c_str(), "rb");
    if (!fp)
        throw std::runtime_error("Failed to open key file: " + path);

    EVP_PKEY* raw = isPrivate
        ? PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr)
        : PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);

    fclose(fp);

    if (!raw)
        throw std::runtime_error("Failed to parse RSA key: " + path);

    _key.reset(raw);
}

EVP_PKEY* RSAKey::get() const noexcept {
    return _key.get();
}

std::vector<uint8_t> generateSessionKey(size_t keySize) {
    std::vector<uint8_t> key(keySize);
    if (!RAND_bytes(key.data(), static_cast<int>(key.size())))
        throw std::runtime_error("RAND_bytes failed");
    return key;
}

std::vector<uint8_t> rsaEncrypt(EVP_PKEY* key, const std::vector<uint8_t>& data) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, nullptr);
    if (!ctx)
        throw std::runtime_error("EVP_PKEY_CTX_new failed");

    if (EVP_PKEY_encrypt_init(ctx) <= 0)
        throw std::runtime_error("EVP_PKEY_encrypt_init failed");

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, data.data(), data.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_encrypt (sizing) failed");

    std::vector<uint8_t> out(outlen);
    if (EVP_PKEY_encrypt(ctx, out.data(), &outlen, data.data(), data.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_encrypt failed");

    EVP_PKEY_CTX_free(ctx);
    return out;
}

std::vector<uint8_t> rsaDecrypt(EVP_PKEY* key, const std::vector<uint8_t>& enc) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, nullptr);
    if (!ctx)
        throw std::runtime_error("EVP_PKEY_CTX_new failed");

    if (EVP_PKEY_decrypt_init(ctx) <= 0)
        throw std::runtime_error("EVP_PKEY_decrypt_init failed");

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, enc.data(), enc.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_decrypt (sizing) failed");

    std::vector<uint8_t> out(outlen);
    if (EVP_PKEY_decrypt(ctx, out.data(), &outlen, enc.data(), enc.size()) <= 0)
        throw std::runtime_error("EVP_PKEY_decrypt failed");

    EVP_PKEY_CTX_free(ctx);
    return out;
}

std::vector<uint8_t> aesEncrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key, std::vector<uint8_t>& iv_out) {
    iv_out.resize(16);
    if (!RAND_bytes(iv_out.data(), static_cast<int>(iv_out.size())))
        throw std::runtime_error("RAND_bytes IV failed");

    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!ctx)
        throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv_out.data()) != 1)
        throw std::runtime_error("EVP_EncryptInit_ex failed");

    std::vector<uint8_t> ciphertext(plaintext.size() + 16);
    int outlen1 = 0;
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &outlen1, plaintext.data(), static_cast<int>(plaintext.size())) != 1)
        throw std::runtime_error("EVP_EncryptUpdate failed");

    int outlen2 = 0;
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + outlen1, &outlen2) != 1)
        throw std::runtime_error("EVP_EncryptFinal_ex failed");

    ciphertext.resize(outlen1 + outlen2);
    return ciphertext;
}

std::vector<uint8_t> aesDecrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv) {
    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!ctx)
        throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1)
        throw std::runtime_error("EVP_DecryptInit_ex failed");

    std::vector<uint8_t> plaintext(ciphertext.size());
    int outlen1 = 0;
    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &outlen1, ciphertext.data(), static_cast<int>(ciphertext.size())) != 1)
        throw std::runtime_error("EVP_DecryptUpdate failed");

    int outlen2 = 0;
    if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + outlen1, &outlen2) != 1)
        throw std::runtime_error("EVP_DecryptFinal_ex failed");

    plaintext.resize(outlen1 + outlen2);
    return plaintext;
}

}  // namespace stx::crypto
}
