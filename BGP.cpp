//
// Created by zach on 08/23/2020.
//

#include <numeric>

#include "BGP.h"
#include "BgpOpenMessage.h"
#include "MessageType.h"
#include "BgpHeader.h"
#include "BgpUpdateMessage.h"
#include "BgpNotificationMessage.h"
#include "FiniteStateMachine.h"
#include "Log.h"

// TODO: track this via user-defined config file (or interactive configuration)
BgpFiniteStateMachine fsm;

class BgpTCPSession : public CppServer::Asio::TCPSession {
public:
//    using TCPSession::TCPSession;

    BgpTCPSession(const std::shared_ptr<CppServer::Asio::TCPServer> &server) : TCPSession(server) {
        fsm.SendMessageToPeer = [=](auto bytes) { SendBytes(this, &bytes[0], bytes.size()); };
    }

protected:

    static void SendBytes(BgpTCPSession* session, void* bytes, size_t size) {
        auto messageBytes = std::vector<uint8_t>(size);
        messageBytes.assign(static_cast<uint8_t*>(bytes), static_cast<uint8_t*>(bytes) + size);
//        std::stringstream message;
//        message << "Sending bytes to peer [" << std::accumulate(messageBytes.begin() + 1, messageBytes.end(), std::to_string(messageBytes[0]), [](const std::string &a, int b) { return a + ',' + std::to_string(b); }) << "]";
//        PrintLogMessage("DEBUG", message.str());
        session->SendAsync(bytes, size);
    }

    void onConnected() override {
        sockaddr_in sa;
        auto addr_string = server()->address();
        inet_pton(AF_INET, addr_string.c_str(), &(sa.sin_addr));
        auto remote_addr = socket().remote_endpoint().address().to_v4().to_uint();
        /* TODO: build a new FSM every time a TCP connection is received/confirmed on port 179
         * fsm_ = BgpFiniteStateMachine(sa.sin_addr.s_addr, remote_addr, 65002, 65001, 16843009, 0, AllowAutomaticStop, {},
                                     {}, {}, {}, {}, {}, {}, [=](auto bytes) { SendBytes(this, &bytes[0], bytes.size()); }, {});;
        fsm_.Start();*/

//        BgpOpenMessage openMessage = {
//                0x04,
//                65002,
//                180,
//                16843009
//        };

        // openMessage.Capabilities.emplace_back(BgpCapability{
        // 	MPBGP, 4, {0x01 /*IPv4 AFI*/, 0x00 /*Reserved*/, 0x01 /*Unicast SAFI*/}
        // });
        //
        // openMessage.Capabilities.emplace_back(BgpCapability{RouteRefreshCapability, 0});
        //
        // openMessage.Capabilities.emplace_back(BgpCapability{EnhancedRouteRefresh, 0});
        //
        // openMessage.Capabilities.emplace_back(BgpCapability{FourByteAsn, 4, {_32to8(65002)}});


//        auto messageHeader = generateBgpHeader(openMessage.GetLength(), MessageType::Open);
//        std::vector<uint8_t> messageBytes(messageHeader.begin(), messageHeader.end());
//        auto messageBody = flattenBgpOpenMessage(openMessage);
//        messageBytes.insert(messageBytes.end(), messageBody.begin(), messageBody.end());
//
//        SendAsync(&messageBytes[0], messageBytes.size());

        std::stringstream message;
        message << "BgpServer connected to peer " << std::to_string((remote_addr >> 24) & 0xFF) << "."
                << std::to_string((remote_addr >> 16) & 0xFF) << "."
                << std::to_string((remote_addr >> 8) & 0xFF) << "."
                << std::to_string(remote_addr & 0xFF) << ".";
        PrintLogMessage("DEBUG", message.str());
        fsm.HandleEvent(TcpConnectionConfirmed);
    }

    void onDisconnected() override {
        std::stringstream message;
        message << "BgpSession with ID " << id() << " was disconnected.";
        PrintLogMessage("INFO", message.str());
        fsm.HandleEvent(AutomaticStop);
    }

    void onReceived(const void *buffer, size_t size) override {
        auto *const bytesBuffer = (uint8_t *) buffer;
        std::vector<uint8_t> messageBytes(bytesBuffer, bytesBuffer + size);
        const std::vector<uint8_t> headerMessageBytes(messageBytes.begin(), messageBytes.begin() + 19);

        const auto header = parseBgpHeader(headerMessageBytes);

        {
            std::stringstream message;
            message << "Received BGP message header with Type " << MessageTypeToString(header.Type) << " and Length "
                    << header.Length;
            PrintLogMessage("INFO", message.str());
        }
        if (header.Length > 19) {
            const std::vector<uint8_t> payloadMessageBytes(messageBytes.begin() + 19, messageBytes.end());
            switch (header.Type) {
                case Open: {
                    auto openMessage = parseBgpOpenMessage(payloadMessageBytes);
                    std::stringstream message;
                    message << "Received BGP OPEN message: " << openMessage.DebugOutput();
                    PrintLogMessage("DEBUG", message.str());
                     fsm.HandleEvent(BgpOpenMessageReceived);
//                    fsm.HandleEvent(TcpConnectionConfirmed);
//                    auto keepalive = generateBgpHeader(0, MessageType::Keepalive);
//                    SendAsync(&keepalive[0], keepalive.size());
                    break;
                }
                case Update: {
                    auto updateMessage = parseBgpUpdateMessage(payloadMessageBytes);
                    std::stringstream message;
                    message << "Received BGP UPDATE message: " << updateMessage.DebugOutput();
                    PrintLogMessage("DEBUG", message.str());
                    fsm.HandleEvent(BgpUpdateMessageReceived);
                    break;
                }
                case Notification: {
                    auto notificationMessage = parseBgpNotificationMessage(payloadMessageBytes);
                    std::stringstream message;
                    message << "Received BGP NOTIFICATION message: " << notificationMessage.DebugOutput();
                    PrintLogMessage("DEBUG", message.str());
                    fsm.HandleEvent(BgpNotificationMessageReceived);
                    break;
                }
                case Keepalive: {
                    std::stringstream message;
                    message << "Keepalive message received with Length > 19. This is not RFC4271-compliant.";
                    PrintLogMessage("ERROR", message.str());
                    break;
                }
                case ReservedMessageType:
                case RouteRefresh:
                default: {
                    std::stringstream message;
                    message << "Unsupported message type " << MessageTypeToString(header.Type);
                    PrintLogMessage("ERROR", message.str());
                    break;
                }
            }
        } else if (header.Length == 19 && header.Type == Keepalive) {
            fsm.HandleEvent(BgpKeepaliveMessageReceived);
        } else {
            std::stringstream message;
            message << "Malformed BGP message received: ["
                      << std::accumulate(messageBytes.begin() + 1, messageBytes.end(), std::to_string(messageBytes[0]),
                                         [](const std::string &a, int b) { return a + ',' + std::to_string(b); })
                      << "]";
            PrintLogMessage("ERROR", message.str());
        }


        {
            std::stringstream message;
            message << "FSM state: " << BgpSessionStateToString(fsm.State);
            PrintLogMessage("DEBUG", message.str());
        }
    }

    void onError(int error, const std::string &category, const std::string &message) override {
        std::stringstream messageString;
        messageString << "BgpSession caught an error with code " << error << " and category '" << category << "': "
                  << message;
        PrintLogMessage("ERROR", messageString.str());
    }

private:
};

class BgpServer : public CppServer::Asio::TCPServer {
public:
    using TCPServer::TCPServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<TCPServer> &server) override {
        return std::make_shared<BgpTCPSession>(server);
    }

    void onError(int error, const std::string &category, const std::string &message) override {
        std::stringstream messageString;
        messageString << "BgpServer caught an error with code " << error << " and category '" << category << "': "
                  << message;
        PrintLogMessage("ERROR", messageString.str());
    }

private:

};

int main() {
    PrintLogMessage("INFO", "Starting BgpServer on port 179.");
    auto service = std::make_shared<CppServer::Asio::Service>();

    PrintLogMessage("INFO", "Starting Asio service...");
    service->Start();
    PrintLogMessage("INFO", "Asio service started.");

    auto server = std::make_shared<BgpServer>(service, 179);

    PrintLogMessage("INFO", "Starting BgpServer...");
    server->Start();
    PrintLogMessage("INFO", "BgpServer started.");

    fsm = BgpFiniteStateMachine(0x0A010166, 0x0A010101, 65002, 65001, 16843009, 0, AllowAutomaticStop, {}, 180, 60, {},
                                {}, {}, {}, nullptr, {});
    fsm.Start();
    fsm.HandleEvent(AutomaticStartWithPassiveTcpEstablishment);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
    }

    PrintLogMessage("INFO", "Stopping BgpServer...");
    server->Stop();
    PrintLogMessage("INFO", "BgpServer stopped.");

    PrintLogMessage("INFO", "Stopping Asio service...");
    service->Stop();
    PrintLogMessage("INFO", "Asio service stopped.");

    return 0;
}
