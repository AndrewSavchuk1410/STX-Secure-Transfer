# STX Secure File Transfer

A C++11-based secure file transfer system using TCP, OpenSSL-based RSA/AES encryption, and resume support.

## 📦 Features

- Mutual RSA-based authentication
- Session key exchange using RSA
- File encryption using AES
- Resume support after sender disconnection
- Windows-only implementation using Winsock and OpenSSL

---

## ⚙️ Build Instructions

### 🧰 Prerequisites

- CMake ≥ 3.20
- MSVC (Visual Studio)
- [vcpkg](https://github.com/microsoft/vcpkg) with `openssl` installed
- Windows (required for Winsock)

### 🏗️ Configure the Project

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/Program Files (x86)/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

> 📝 Adjust the path to the vcpkg toolchain file if different on your system.

### 🧱 Build (Debug or Release)

```bash
cmake --build build --config Release
# or for Debug:
cmake --build build --config Debug
```

This builds the following executables:
- `stx-send.exe` — located in `build/stx-send/Release/` or `Debug/`
- `stx-recv.exe` — located in `build/stx-recv/Release/` or `Debug/`
- `crypto_tests.exe` — located in `build/Release/` or `Debug/`

---

## 🔐 RSA Keys

Ensure the following RSA key files exist in the `keys/` directory:

- `send_priv.pem` — private key for the sender
- `send_pub.pem`  — public key for the sender
- `recv_priv.pem` — private key for the receiver
- `recv_pub.pem`  — public key for the receiver

Use OpenSSL to generate them if needed:
```bash
openssl genrsa -out send_priv.pem 2048
openssl rsa -in send_priv.pem -pubout -out send_pub.pem

openssl genrsa -out recv_priv.pem 2048
openssl rsa -in recv_priv.pem -pubout -out recv_pub.pem
```

---

## ✅ Running Tests

### 🧪 Unit Tests

To run the encryption/decryption unit tests:

```bash
C:\Projects\test_task\build\Release\crypto_tests.exe
```

> Replace `Release` with `Debug` if you built in Debug mode.

### 🔁 Integration Tests

Integration tests simulate full end-to-end transfers.

Run them from the `tests/` directory:

```bash
cd C:\Projects\test_task\tests
python integration_test.py
```

You should see output such as:
```
[TEST] Standard 5MB transfer
[PASS] Files match

All integration tests passed.
```

---

## 📁 Project Structure

```
/common/         # Core encryption/network/transfer logic
/stx-send/       # Sender CLI
/stx-recv/       # Receiver CLI
/tests/          # Integration tests & test files
/keys/           # RSA key files
build/           # CMake build output
```

---

## 🛠️ Example Usage

### Start receiver:
```bash
build\stx-recv\Release\stx-recv.exe --listen 9000 --out C:\Downloads\
```

### Send a file:
```bash
build\stx-send\Release\stx-send.exe 127.0.0.1 9000 C:\Files\example.bin --key keys\send_priv.pem
```

---

