//
// Created by zach on 2021-04-17.
//

#ifndef BGP_SERVERSOCKET_H
#define BGP_SERVERSOCKET_H

#include <utility>
#include <iostream>

#include "Socket.h"
#include "Log.h"

class ServerSocket : public TcpSocket {
public:
    // TODO: think through what I really want to expose to the user - just a constructor w/ a port? Or make the user just pass in a SocketAddress?
    // TODO: do I want to split this out into IPv4ServerSocket and IPv6ServerSocket like IPv4Address and IPv6Address? I'd prefer not to, but it might make things easier.
    explicit ServerSocket(std::shared_ptr<SocketAddress> address) : TcpSocket(address) {

        // TODO: this definitely doesn't work for IPv6...
        auto addr = reinterpret_cast<const in_addr*>(address_->addr()->bytes());
        const sockaddr_in saddr = {
                .sin_family = static_cast<uint16_t>(address_->addr()->family()),
                .sin_port = address_->port(),
                .sin_addr = *addr
        };
        // TODO: this might not work...
        // TODO: spoiler alert, it didn't
        auto bindResult = bind(socketHandle_, reinterpret_cast<const sockaddr*>(&saddr), address->length());

        if (bindResult != 0) {
            logging::sockets::ERROR("ServerSocket::ServerSocket()::bind()");
            // TODO: error handling
        }

        // TODO: figure out what # is best for backlog
        constexpr auto BACKLOG = 10;
        auto listenResult = listen(socketHandle_, BACKLOG);
        if (listenResult == SOCKET_ERROR) {
            logging::sockets::ERROR("ServerSocket::ServerSocket()::listen()");
            // TODO: error handling
        }
    }

    // TODO: multithreaded Accept()
    std::shared_ptr<TcpSocket> Accept() {
        sockaddr_storage remoteAddressInfo{};
        socklen_t remoteAddressSize = sizeof(remoteAddressInfo);
        const auto remoteAddress = reinterpret_cast<sockaddr *>(&remoteAddressInfo);
        auto acceptedSocketHandle = accept(socketHandle_, remoteAddress, &remoteAddressSize);
        if (acceptedSocketHandle == INVALID_SOCKET) {
            logging::sockets::ERROR("ServerSocket::Accept()::accept()");
            // TODO: error handling
        }

//    sockaddr remoteAddress{};
//    int remoteAddressLength = sizeof(remoteAddress);
//    auto getPeerNameResult = getpeername(acceptedSocketHandle, &remoteAddress, &remoteAddressLength);
//    if (getPeerNameResult == SOCKET_ERROR) {
//        // TODO: error handling
//    }

        std::shared_ptr<IPAddress> address;
        uint16_t port;

        if (remoteAddressInfo.ss_family == AF_INET) {
            const auto remoteAddressIPv4 = reinterpret_cast<sockaddr_in*>(remoteAddress);
            address = std::make_shared<IPv4Address>(remoteAddressIPv4->sin_addr);
            port = remoteAddressIPv4->sin_port;
        }
        else if (remoteAddressInfo.ss_family == AF_INET6) {
            const auto remoteAddressIPv6 = reinterpret_cast<sockaddr_in6*>(remoteAddress);
            address = std::make_shared<IPv6Address>(remoteAddressIPv6->sin6_addr);
            port = remoteAddressIPv6->sin6_port;
        }
        else {
            // TODO: error handling
        }

        auto socketAddress = std::make_shared<SocketAddress>(address, port);
        return std::make_shared<TcpSocket>(socketAddress, acceptedSocketHandle);
    }
};

#endif //BGP_SERVERSOCKET_H
