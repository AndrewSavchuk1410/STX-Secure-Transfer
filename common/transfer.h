#pragma once

#include "crypto.h"
#include "socket.h"
#include <fstream>
#include <vector>
#include <cstdint>

namespace stx {
namespace transfer {

struct FileMetadata {
    std::string filename;
    uint64_t filesize;
    uint64_t lastConfirmedBlock;

    std::string to_string() const;
    static FileMetadata from_string(const std::string& str);
};

bool sendMetadata(SOCKET sock, const FileMetadata& meta);
bool recvMetadata(SOCKET sock, FileMetadata& out);

bool sendEncryptedBlock(SOCKET sock, const std::vector<uint8_t>& block, const std::vector<uint8_t>& key);
bool recvEncryptedBlock(SOCKET sock, std::vector<uint8_t>& block, const std::vector<uint8_t>& key);

bool sendIV(SOCKET sock, const std::vector<uint8_t>& iv);
bool recvIV(SOCKET sock, std::vector<uint8_t>& iv);

bool sendUInt64(SOCKET sock, uint64_t value);
bool recvUInt64(SOCKET sock, uint64_t& value);

bool sendAll(SOCKET sock, const char* buf, size_t len);
bool recvAll(SOCKET sock, char* buf, size_t len);

} 
}
