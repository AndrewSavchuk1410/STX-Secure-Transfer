# STX Secure Transfer – Design Overview

This document outlines the architecture, protocol, threat model, and implementation characteristics of the STX secure file transfer utility.

---

## 🔐 Protocol Implementation

### 2.1 Handshake

| Feature                            | Implemented | Explanation |
|-----------------------------------|-------------|-------------|
| **Session Key Generation**        | ✅ Yes      | The sender generates a 256-bit AES session key using `RAND_bytes` and encrypts it using the receiver's RSA public key. |
| **Mutual Authentication**         | ❌ No       | Currently only one-way: the sender ensures the receiver is authentic by encrypting the session key with their known public key. |
|                                   |             | To implement mutual authentication, a challenge-response mechanism can be added. The receiver sends a random nonce; the sender signs it using their private RSA key. The receiver verifies the signature using the sender's public key. |

---

### 2.2 Data Transfer

| Feature               | Implemented | Explanation |
|----------------------|-------------|-------------|
| **AES Encryption**    | ✅ Yes      | Each block is encrypted using AES-256-CBC with a fresh IV generated using `RAND_bytes`. |
| **Integrity Checking**| ❌ No       | No authentication tags or checksums are present to detect tampering. |
|                       |             | To ensure data integrity, AES-GCM can be used instead of CBC. Alternatively, an HMAC (e.g., HMAC-SHA256) can be calculated and transmitted alongside each block and verified before decryption. |

---

### 2.3 Resume Support

| Feature                                | Implemented | Explanation |
|---------------------------------------|-------------|-------------|
| **Resuming After Disconnection**      | ✅ Yes      | After reconnecting, the receiver communicates the last successfully written block, and the sender resumes transmission from that point. |
| **File Identity Verification**        | ❌ No       | Currently, there is no validation that the file being resumed is the same as the one originally transferred. |
|                                       |             | To verify identity during resume, a hash (e.g., SHA-256) of the file or its first N KB can be included in the metadata. The receiver can store and compare this hash upon reconnection. |

---

## 🛡 Threat Model

### Security Goals

- **Confidentiality**: Achieved using AES-256 encryption
- **Authentication**: One-way (receiver is authenticated by the sender)
- **Integrity**: Not cryptographically guaranteed — only assumed through successful decryption

### Threat Analysis

| Threat                 | Mitigated | Explanation |
|------------------------|-----------|-------------|
| Eavesdropping          | ✅ Yes    | All file data is encrypted with AES-256 |
| Man-in-the-Middle      | ❌ No     | Pre-shared keys are used, but no session proof or signatures |
| Replay Attack          | ❌ No     | No session ID, nonce, or timestamp is used |
| IV Reuse (AES-CBC)     | ✅ Yes    | IV is freshly generated for each block using `RAND_bytes` |
| Session Hijacking      | ✅ Yes    | All data after the handshake is bound to the session key |

To mitigate remaining threats:
- For man-in-the-middle protection, an RSA-based challenge-response protocol can be introduced.
- To prevent replay attacks, session metadata can include a nonce or timestamp and be verified by the receiver.
- To detect tampering, authenticated encryption (e.g., AES-GCM) or HMAC can be introduced to validate each block.

---

## ⚙️ Non-Functional Requirements

| Requirement                               | Implemented | Explanation |
|-------------------------------------------|-------------|-------------|
| **C++11 + CMake ≥ 3.20**                  | ✅ Yes      | The system is written in C++11 and uses modern CMake project configuration. |
| **OpenSSL**                               | ✅ Yes      | Used for cryptographic primitives (RSA, AES, IVs, secure RNG). |
| **Compiler Flags: -Wall -Wextra -Werror** | ✅ Yes  | Enabled to enforce strict warning and error checking. |
| **RAII and Smart Pointers**               | ✅ Yes      | All key resources are wrapped in RAII classes (`SocketRAII`, `EVP_PKEY_ptr`, etc.). |
| **Use of `noexcept`**                     | ⚠️ Partial  | Some methods are marked `noexcept`, but coverage is not yet comprehensive. |
| **Error output via stderr**               | ✅ Yes      | All runtime errors are logged to `std::cerr` via `log_error`. |
| **Exit Codes (0–3)**                      | ✅ Yes      | Standardized exit codes represent success, network error, authentication failure, and I/O error. |
| **Concurrent Client Support**             | ❌ No       | The receiver handles a single client in blocking mode. |
|                                           |             | To support multiple clients, each accepted connection can be handled in a separate `std::thread`, or a non-blocking model can be introduced using `select()`, `epoll`, or `boost::asio`. |

---

## 📌 Summary

The STX secure transfer protocol provides reliable encrypted file transmission with session resumption support and a strong foundation in AES and RSA cryptography. The current implementation satisfies key confidentiality and resumability goals but does not yet enforce mutual authentication, integrity verification, or concurrent session handling. These features can be incrementally introduced without major architectural changes.
