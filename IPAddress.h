//
// Created by zach on 2021-04-17.
//

#ifndef BGP_IPADDRESS_H
#define BGP_IPADDRESS_H

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

enum class AddressFamily {
    IPv4 = AF_INET,
    IPv6 = AF_INET6
};

class IPAddress {
public:
    virtual std::string to_string() const = 0;

    virtual AddressFamily family() const = 0;

    virtual const void *bytes() const = 0;
};

class IPv4Address : public IPAddress {
public:
    IPv4Address() = default;

    explicit IPv4Address(const in_addr address) : address_(address) {}

    explicit IPv4Address(const std::string &address) {
        inet_pton(static_cast<int>(family()), address.c_str(), &address_);
    }

    std::string to_string() const override {
        char tmp[INET_ADDRSTRLEN];
        inet_ntop(static_cast<int>(family()), &address_, tmp, sizeof(tmp));
        return tmp;
    }

    AddressFamily family() const override {
        return AddressFamily::IPv4;
    }

    const void *bytes() const override {
        return reinterpret_cast<const void *>(&address_);
    }

private:
    in_addr address_;
};

class IPv6Address : public IPAddress {
public:
    IPv6Address() = default;

    explicit IPv6Address(const in6_addr address) : address_(address) {

    }

    explicit IPv6Address(const std::string &address) {
        inet_pton(static_cast<int>(family()), address.c_str(), &address_);
    }

    std::string to_string() const override {
        char tmp[INET6_ADDRSTRLEN];
        inet_ntop(static_cast<int>(family()), &address_, tmp, sizeof(tmp));
        return tmp;
    }

    AddressFamily family() const override {
        return AddressFamily::IPv6;
    }

    const void *bytes() const override {
        return reinterpret_cast<const void *>(&address_);
    }

private:
    in6_addr address_;
};

#endif //BGP_IPADDRESS_H
