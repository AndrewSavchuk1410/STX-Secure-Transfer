extern "C" {
#include <openssl/applink.c>
}

#include "../common/crypto.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

using namespace stx::crypto;

// Вивід даних у hex
void print_hex(const std::vector<uint8_t>& data, const std::string& label) {
    std::cout << label << ": ";
    for (uint8_t b : data)
        std::printf("%02X ", b);
    std::cout << "\n";
}

void test_aes_encrypt_decrypt() {
    std::vector<uint8_t> key = generateSessionKey(32);
    std::vector<uint8_t> iv;
    std::vector<uint8_t> plaintext = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};

    auto ciphertext = aesEncrypt(plaintext, key, iv);
    auto decrypted  = aesDecrypt(ciphertext, key, iv);

    assert(plaintext == decrypted);
    std::cout << "[PASS] AES encrypt/decrypt\n";
}

void test_rsa_encrypt_decrypt() {
    RSAKey priv("../../keys/recv_priv.pem", true); 
    RSAKey pub("../../keys/recv_pub.pem", false); 

    std::vector<uint8_t> message = {'S', 'e', 'c', 'r', 'e', 't'};
    auto encrypted = rsaEncrypt(pub.get(), message);
    auto decrypted = rsaDecrypt(priv.get(), encrypted);

    //print_hex(message,   "Original");
    //print_hex(encrypted, "Encrypted");
    //print_hex(decrypted, "Decrypted");

    // Обрізай зайві null-байти (не завжди, але гарантовано)
    decrypted.resize(message.size());

    if (message == decrypted) {
        std::cout << "[PASS] RSA encrypt/decrypt\n";
    } else {
        std::cerr << "[FAIL] RSA decrypt result != original\n";
        std::exit(1);
    }
}

void test_session_key_generation() {
    auto key1 = generateSessionKey();
    auto key2 = generateSessionKey();

    assert(key1.size() == 32);
    assert(key2.size() == 32);
    assert(key1 != key2);
    std::cout << "[PASS] Session key generation\n";
}

int main() {
    try {
        test_session_key_generation();
        test_aes_encrypt_decrypt();
        test_rsa_encrypt_decrypt();
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Exception: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
