//
// Created by zach on 2021-04-17.
//

#ifndef BGP_SOCKET_H
#define BGP_SOCKET_H

#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <algorithm>

#include "SocketAddress.h"
#include "Log.h"

enum class SocketType {
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
};

class Socket {
public:
    explicit Socket(std::shared_ptr<SocketAddress> address, const SocketType type) : address_(std::move(address)) {
        // TODO: implement this better, this isn't going to work well for most cases, or at all for IPv6. IPv6 support is not optional in [[current year]]
        // going to forgo getaddrinfo() in favor of manually filling out structs

        auto addr = address_->addr();
        socketHandle_ = socket(static_cast<int>(addr->family()), static_cast<int>(type), NULL);
        if (socketHandle_ == INVALID_SOCKET) {
            logging::sockets::ERROR("Socket::Socket()::socket()");
            // TODO: error handling
        }
    }

    explicit Socket(std::shared_ptr<SocketAddress> address, const int socketHandle) : address_(std::move(address)), socketHandle_(socketHandle) {

    }

    ~Socket() {
        Close();
    }

    std::shared_ptr<SocketAddress> address() {
        return address_;
    }

    virtual SocketType type() const = 0;

    virtual std::string to_string() const = 0;

    void Close() const {
        if (closesocket(socketHandle_) != 0) {
            logging::sockets::ERROR("Socket::Close()::closesocket()");
            // TODO: error handling
        }
    }

    void Send(const std::vector<uint8_t>& data) const {
        const auto dataTransformed = convertToSigned(data);
        auto bytesSent = send(socketHandle_, dataTransformed.data(), static_cast<int>(dataTransformed.size()), NULL);
        if (bytesSent == -1) {
            logging::sockets::ERROR("Socket::Send()::send()");
        }
        while (bytesSent < dataTransformed.size()) {
            auto bytesToSend = static_cast<int>(dataTransformed.size()) - bytesSent;
            bytesSent += send(socketHandle_, dataTransformed.data() + bytesSent, bytesToSend, NULL);
        }
    }

    std::vector<uint8_t> Receive() const {
        constexpr auto RECEIVE_BUFFER_SIZE = 4096;
        char receiveBuffer[RECEIVE_BUFFER_SIZE];
        auto bytesReceived = recv(socketHandle_, receiveBuffer, RECEIVE_BUFFER_SIZE, NULL);
        if (bytesReceived == -1) {
            logging::sockets::ERROR("Socket::Receive()::recv()");
        }
        else if (bytesReceived == 0) {
            std::stringstream message;
            message << to_string() << " - Socket::Receive()::recv() - remote end closed connection." << std::endl;
            logging::WARN(message.str());
        }
        else {
            return convertToUnsigned(std::vector<char>(receiveBuffer, receiveBuffer + bytesReceived));
        }
        return {};
    }
protected:
    std::shared_ptr<SocketAddress> address_;
    int socketHandle_;

private:
    static std::vector<char> convertToSigned(const std::vector<uint8_t>& vector) {
        std::vector<char> result(vector.size());
        std::transform(vector.begin(), vector.end(), result.begin(), [](uint8_t input) -> int8_t {
            return static_cast<char>((input >= (1 << 7)) ? -(UINT8_MAX - input + 1) : input);
        });
        return result;
    }

    static std::vector<uint8_t> convertToUnsigned(const std::vector<char>& vector) {
        std::vector<uint8_t> result(vector.size());
        std::transform(vector.begin(), vector.end(), result.begin(), [](char input) -> uint8_t {
            return static_cast<uint8_t>(input);
        });
        return result;
    }

};

class TcpSocket : public Socket {
public:
    explicit TcpSocket(std::shared_ptr<SocketAddress> address) : Socket(std::move(address), SocketType::TCP) {

    }

    explicit TcpSocket(std::shared_ptr<SocketAddress> address, int socketHandle) : Socket(std::move(address), socketHandle) {

    }

    // TODO: do I need this outside the Socket::Socket() constructor?
    SocketType type() const override {
        return SocketType::TCP;
    }

    std::string to_string() const override {
        std::stringstream result;
        result << "TcpSocket connected to " << address_->to_string();
        return result.str();
    }
};

class UdpSocket : public Socket {
public:
    explicit UdpSocket(std::shared_ptr<SocketAddress> address) : Socket(address, SocketType::UDP) {

    }

    // TODO: do I need this outside the Socket::Socket() constructor?
    SocketType type() const override {
        return SocketType::UDP;
    }

    std::string to_string() const override {
        std::stringstream result;
        result << "UdpSocket connected to " << address_->to_string();
        return result.str();
    }
};

#endif //BGP_SOCKET_H
