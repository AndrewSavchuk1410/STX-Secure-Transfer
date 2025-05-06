#include "socket.h"
#include <stdexcept>
#include <iostream>

namespace stx {
namespace net {

SocketRAII::SocketRAII() {
    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sock == INVALID_SOCKET)
        throw std::runtime_error("Failed to create socket");
}

SocketRAII::SocketRAII(SocketRAII&& other) noexcept
  : _sock(other._sock)
{
    other._sock = INVALID_SOCKET;
}

// move-assignment
SocketRAII& SocketRAII::operator=(SocketRAII&& other) noexcept
{
    if (this != &other) {
        close();
        _sock = other._sock;
        other._sock = INVALID_SOCKET;
    }
    return *this;
}

SocketRAII::~SocketRAII() {
    close();
}

void SocketRAII::close() noexcept {
    if (_sock != INVALID_SOCKET) {
        closesocket(_sock);
        _sock = INVALID_SOCKET;
    }
}

SOCKET SocketRAII::get() const noexcept {
    return _sock;
}

void SocketRAII::connectTo(const std::string& host, uint16_t port) {
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0)
        throw std::runtime_error("Invalid address");

    if (connect(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        throw std::runtime_error("Failed to connect");
}

void SocketRAII::bindAndListen(uint16_t port) {
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        throw std::runtime_error("Bind failed");

    if (listen(_sock, SOMAXCONN) == SOCKET_ERROR)
        throw std::runtime_error("Listen failed");
}

SocketRAII SocketRAII::acceptClient() {
    SOCKET client = accept(_sock, nullptr, nullptr);
    if (client == INVALID_SOCKET)
        throw std::runtime_error("Accept failed");

    SocketRAII result;
    result._sock = client;
    return result;
}

void initWinsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        throw std::runtime_error("WSAStartup failed");
}

void cleanupWinsock() {
    WSACleanup();
}

}  // namespace stx::net
}
