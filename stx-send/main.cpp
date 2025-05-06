extern "C" {
#include <openssl/applink.c>
}

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

#include "../common/socket.h"
#include "../common/crypto.h"
#include "../common/transfer.h"
#include "../common/common.h"

using namespace stx;

int main(int argc, char* argv[]) {
    if (argc < 6 || std::string(argv[4]) != "--key") {
        std::cerr << "Usage: stx-send <host> <port> <file> --key <privkey.pem>\n";
        return ExitCode::IO_ERROR;
    }

    const std::string host      = argv[1];
    const uint16_t    port      = static_cast<uint16_t>(std::stoi(argv[2]));
    const std::string filepath  = argv[3];
    const std::string privkey   = argv[5];

    std::ifstream infile(filepath, std::ios::binary);
    if (!infile) {
        log_error("Cannot open file: " + filepath);
        return ExitCode::IO_ERROR;
    }
    infile.seekg(0, std::ios::end);
    auto pos = infile.tellg();
    if (pos < 0) {
        log_error("Failed to get file size");
        return ExitCode::IO_ERROR;
    }
    const uint64_t filesize = static_cast<uint64_t>(pos);
    infile.seekg(0);

    const uint64_t totalBlocks = (filesize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    uint64_t sentBlocks = 0;

    net::initWinsock();
    while (sentBlocks < totalBlocks) {
        try {
            net::SocketRAII sock;
            sock.connectTo(host, port);

            crypto::RSAKey    myPriv(privkey, true);
            crypto::RSAKey    peerPub("../../../keys/recv_pub.pem", false);
            auto              sessionKey   = crypto::generateSessionKey();
            auto              encryptedKey = crypto::rsaEncrypt(peerPub.get(), sessionKey);

            transfer::sendUInt64(sock.get(), encryptedKey.size());
            send(sock.get(),
                    reinterpret_cast<const char*>(encryptedKey.data()),
                    static_cast<int>(encryptedKey.size()),
                    0);

            transfer::FileMetadata meta;
            meta.filename           = filepath.substr(filepath.find_last_of("/\\") + 1);
            meta.filesize           = filesize;
            meta.lastConfirmedBlock = 0;
            transfer::sendMetadata(sock.get(), meta);

            uint64_t resumeFrom = 0;
            if (!transfer::recvUInt64(sock.get(), resumeFrom))
                throw std::runtime_error("Failed to receive resume offset");
            sentBlocks = resumeFrom;
            infile.clear();
            infile.seekg(sentBlocks * BLOCK_SIZE);

            std::vector<uint8_t> buf(BLOCK_SIZE);
            while (!infile.eof()) {
                infile.read(reinterpret_cast<char*>(buf.data()), BLOCK_SIZE);
                std::streamsize r = infile.gcount();
                if (r <= 0) break;
                buf.resize(static_cast<size_t>(r));

                if (!transfer::sendEncryptedBlock(sock.get(), buf, sessionKey))
                    throw std::runtime_error("Failed at block " + std::to_string(sentBlocks));

                ++sentBlocks;
                buf.resize(BLOCK_SIZE);
            }

            transfer::sendUInt64(sock.get(), 0);
            shutdown(sock.get(), SD_SEND);

            std::cout << "Transfer complete: " << sentBlocks << " / " << totalBlocks << " blocks\n";
            break;
        }
        catch (const std::exception& ex) {
            log_error(ex.what());
            std::cerr << "[WARN] Disconnected at block " << sentBlocks
                        << ", retrying in 1s...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    net::cleanupWinsock();
    return ExitCode::SUCCESS;
}
