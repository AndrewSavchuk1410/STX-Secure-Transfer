extern "C" {
#include <openssl/applink.c>
}

#include <iostream>
#include <fstream>
#include <direct.h>
#include <sys/stat.h>

#include "../common/socket.h"
#include "../common/crypto.h"
#include "../common/transfer.h"
#include "../common/common.h"

using namespace stx;

static void ensure_directory_exists(const std::string& path) {
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0)
        _mkdir(path.c_str());
}

int main(int argc, char* argv[]) {
    if (argc < 5 || std::string(argv[1]) != "--listen" || std::string(argv[3]) != "--out") {
        std::cerr << "Usage: stx-recv --listen <port> --out <output_dir>\n";
        return ExitCode::IO_ERROR;
    }

    uint16_t    port   = static_cast<uint16_t>(std::stoi(argv[2]));
    std::string outdir = argv[4];

    net::initWinsock();
    crypto::RSAKey myPriv("../../../keys/recv_priv.pem", true);

    net::SocketRAII server;
    server.bindAndListen(port);
    std::cout << "Listening on port " << port << "...\n";

    while (true) {
        try {
            auto client = server.acceptClient();
            std::cout << "Client connected.\n";

            uint64_t keyLen = 0;
            if (!transfer::recvUInt64(client.get(), keyLen))
                throw std::runtime_error("Failed to read key length");

            std::vector<uint8_t> encryptedKey(keyLen);
            if (!transfer::recvAll(client.get(),
                                    reinterpret_cast<char*>(encryptedKey.data()),
                                    keyLen))
                throw std::runtime_error("Failed to read key data");

            auto sessionKey = crypto::rsaDecrypt(myPriv.get(), encryptedKey);

            transfer::FileMetadata meta;
            if (!transfer::recvMetadata(client.get(), meta))
                throw std::runtime_error("Metadata receive failed");
            std::cout << "Metadata: file=" << meta.filename
                        << " size=" << meta.filesize << "\n";

            ensure_directory_exists(outdir);
            std::string outPath = outdir + "/" + meta.filename;
            std::ofstream outfile(outPath, std::ios::binary | std::ios::app);
            if (!outfile)
                throw std::runtime_error("Cannot open " + outPath);

            uint64_t lastBlock = uint64_t(outfile.tellp()) / BLOCK_SIZE;
            transfer::sendUInt64(client.get(), lastBlock);

            std::vector<uint8_t> block;
            uint64_t count = lastBlock;
            while (transfer::recvEncryptedBlock(client.get(), block, sessionKey)) {
                outfile.write(reinterpret_cast<char*>(block.data()), block.size());
                ++count;
            }
            std::cout << "Received " << count << " blocks\n";
        }
        catch (const std::exception& ex) {
            log_error(ex.what());
            std::cerr << "Session ended, waiting for next client...\n";
        }
    }

    net::cleanupWinsock();
    return ExitCode::SUCCESS;
}
