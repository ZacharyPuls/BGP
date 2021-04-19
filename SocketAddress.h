//
// Created by zach on 2021-04-17.
//

#ifndef BGP_SOCKETADDRESS_H
#define BGP_SOCKETADDRESS_H

#include <memory>
#include <sstream>
#include <utility>

#include "IPAddress.h"
#include "Log.h"

class SocketAddress {
public:
    // TODO: fix length_
    SocketAddress(std::shared_ptr<IPAddress> address, uint16_t port) : address_(std::move(address)), port_(port), length_(128) { }

    explicit SocketAddress(const std::string& address, const std::string& port) {
        addrinfo aiHints = {0};
        addrinfo* addressInfo;

        // TODO: does this work for IPv6?
        aiHints.ai_family = AF_INET;
        aiHints.ai_socktype = SOCK_STREAM;
        aiHints.ai_flags = AI_PASSIVE;

        auto nodeName = address.empty() ? nullptr : address.c_str();
        auto gaiResult = getaddrinfo(nodeName, port.c_str(), &aiHints, &addressInfo);
        if (gaiResult != 0) {
            logging::sockets::ERROR("SocketAddress::SocketAddress()::getaddrinfo()");
            // TODO: error handling
        }

        length_ = static_cast<int>(addressInfo->ai_addrlen);

        // TODO: actually walk through the linked list (addressInfo->ai_next)
        switch (addressInfo->ai_family) {
            case AF_INET: {
                auto addr = reinterpret_cast<sockaddr_in *>(addressInfo->ai_addr);
                address_ = std::make_shared<IPv4Address>(addr->sin_addr);
                port_ = addr->sin_port;
                break;
            }
            case AF_INET6: {
                auto addr = reinterpret_cast<sockaddr_in6 *>(addressInfo->ai_addr);
                address_ = std::make_shared<IPv6Address>(addr->sin6_addr);
                port_ = addr->sin6_port;
                break;
            }
            default:
                // TODO: error handling
                break;
        }
    }

    std::shared_ptr<IPAddress> addr() const {
        return address_;
    }

    uint16_t port() const {
        return port_;
    }

    int length() const {
        return length_;
    }

    std::string to_string() const {
        std::stringstream result;
        result << address_->to_string() << ':' << ntohs(port_);
        return result.str();
    }
private:
    std::shared_ptr<IPAddress> address_;
    uint16_t port_;
    int length_;
};

#endif //BGP_SOCKETADDRESS_H
