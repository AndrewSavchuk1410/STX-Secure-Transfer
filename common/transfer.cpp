#include "transfer.h"
#include "common.h"
#include <winsock2.h>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace stx {
namespace transfer {

std::string FileMetadata::to_string() const {
    return filename + "|" + std::to_string(filesize) + "|" + std::to_string(lastConfirmedBlock);
}

FileMetadata FileMetadata::from_string(const std::string& str) {
    FileMetadata meta;
    size_t p1 = str.find('|');
    size_t p2 = str.find('|', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos)
        throw std::runtime_error("Invalid metadata format");

    meta.filename = str.substr(0, p1);
    meta.filesize = std::stoull(str.substr(p1 + 1, p2 - p1 - 1));
    meta.lastConfirmedBlock = std::stoull(str.substr(p2 + 1));
    return meta;
}

bool sendMetadata(SOCKET sock, const FileMetadata& meta) {
    std::string data = meta.to_string();
    uint64_t len = data.size();
    if (!sendUInt64(sock, len))
        return false;
    return send(sock, data.c_str(), static_cast<int>(data.size()), 0) == data.size();
}

bool recvMetadata(SOCKET sock, FileMetadata& out) {
    uint64_t len;
    if (!recvUInt64(sock, len))
        return false;

    std::string buf(len, '\0');
    int received = recv(sock, &buf[0], static_cast<int>(len), MSG_WAITALL);
    if (received != len)
        return false;

    out = FileMetadata::from_string(buf);
    return true;
}

bool sendEncryptedBlock(SOCKET sock, const std::vector<uint8_t>& block, const std::vector<uint8_t>& key) {
    std::vector<uint8_t> iv;
    auto encrypted = crypto::aesEncrypt(block, key, iv);

    if (!sendIV(sock, iv)) return false;
    return sendUInt64(sock, encrypted.size()) &&
           send(sock, reinterpret_cast<const char*>(encrypted.data()), static_cast<int>(encrypted.size()), 0) == encrypted.size();
}

bool recvEncryptedBlock(SOCKET sock, std::vector<uint8_t>& block, const std::vector<uint8_t>& key) {
    uint64_t ivlen = 0;
    if (!recvUInt64(sock, ivlen)) 
        return false;       

    if (ivlen == 0)
        return false;      

    std::vector<uint8_t> iv(ivlen);
    if (!recvAll(sock, reinterpret_cast<char*>(iv.data()), ivlen))
        return false;     

    uint64_t enclen = 0;
    if (!recvUInt64(sock, enclen))
        return false;

    std::vector<uint8_t> enc(enclen);
    if (!recvAll(sock, reinterpret_cast<char*>(enc.data()), enclen))
        return false;

    block = crypto::aesDecrypt(enc, key, iv);
        return true;
}


bool sendIV(SOCKET sock, const std::vector<uint8_t>& iv) {
    uint64_t len = iv.size();
    return sendUInt64(sock, len) &&
           send(sock, reinterpret_cast<const char*>(iv.data()), static_cast<int>(len), 0) == len;
}

bool recvIV(SOCKET sock, std::vector<uint8_t>& iv) {
    uint64_t len;
    if (!recvUInt64(sock, len)) return false;
    iv.resize(len);
    int got = recv(sock, reinterpret_cast<char*>(iv.data()), static_cast<int>(len), MSG_WAITALL);
    return got == len;
}

bool sendUInt64(SOCKET sock, uint64_t value) {
    uint64_t net = htonll(value);
    return sendAll(sock, reinterpret_cast<const char*>(&net), sizeof(net));
}

bool recvUInt64(SOCKET sock, uint64_t& value) {
    uint64_t net = 0;
    if (!recvAll(sock, reinterpret_cast<char*>(&net), sizeof(net))) {
        log_error("recvUInt64: failed to read 8 bytes");
        return false;
    }
    value = ntohll(net);
    return true;
}

bool sendAll(SOCKET sock, const char* buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        int r = ::send(sock, buf + sent, int(len - sent), 0);
        if (r == SOCKET_ERROR) return false;
        sent += r;
    }
    return true;
}

bool recvAll(SOCKET sock, char* buf, size_t len) {
    size_t recvd = 0;
    while (recvd < len) {
        int r = ::recv(sock, buf + recvd, int(len - recvd), MSG_WAITALL);
        if (r == 0) {
            return false;
        }
        if (r == SOCKET_ERROR) {
            log_error("recvAll: WSA error " + std::to_string(WSAGetLastError()));
            return false;
        }
        recvd += r;
    }
    return true;
}

} 
}
