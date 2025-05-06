#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

namespace stx {
namespace net {

class SocketRAII {
public:
    SocketRAII();
    ~SocketRAII();

    SocketRAII(const SocketRAII&) = delete;
    SocketRAII& operator=(const SocketRAII&) = delete;

    // enable move
    SocketRAII(SocketRAII&& other) noexcept;
    SocketRAII& operator=(SocketRAII&& other) noexcept;

    SOCKET get() const noexcept;
    void close() noexcept;

    void connectTo(const std::string& host, uint16_t port);
    void bindAndListen(uint16_t port);
    SocketRAII acceptClient();

private:
    SOCKET _sock;
};
        

void initWinsock();
void cleanupWinsock();

} 
}
