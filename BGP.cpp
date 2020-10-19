//
// Created by zach on 08/23/2020.
//

#include <numeric>
#include <iomanip>

#include "BGP.h"
#include "BgpOpenMessage.h"
#include "MessageType.h"
#include "BgpHeader.h"
#include "BgpUpdateMessage.h"
#include "BgpNotificationMessage.h"
#include "FiniteStateMachine.h"
#include "Log.h"

class BgpTCPSession : public CppServer::Asio::TCPSession {
public:
//    using TCPSession::TCPSession;

    BgpTCPSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server, std::shared_ptr<BgpFiniteStateMachine>& fsm) : TCPSession(server), fsm_(fsm) {
        fsm_->SendMessageToPeer = [=](auto bytes) { SendBytes(this, &bytes[0], bytes.size()); };
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
        logging::DEBUG(message.str());
        fsm_->HandleEvent(TcpConnectionConfirmed);
    }

    void onDisconnected() override {
        std::stringstream message;
        message << "BgpSession with ID " << id() << " was disconnected.";
        logging::INFO(message.str());
        fsm_->HandleEvent(AutomaticStop);
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
            logging::INFO(message.str());
        }
        if (header.Length > 19) {
            // TODO: I have no clue why I'm receiving TCP segments from my home router with a bunch of garbage appended to the end, but in every case, the length is correct in the header. Should look into this in the future.
            const std::vector<uint8_t> payloadMessageBytes(messageBytes.begin() + 19, messageBytes.begin() + header.Length);
            switch (header.Type) {
                case Open: {
                    auto openMessage = parseBgpOpenMessage(payloadMessageBytes);
                    std::stringstream message;
                    message << "Received BGP OPEN message: " << openMessage.DebugOutput();
                    logging::DEBUG(message.str());
                     fsm_->HandleEvent(BgpOpenMessageReceived);
//                    fsm.HandleEvent(TcpConnectionConfirmed);
//                    auto keepalive = generateBgpHeader(0, MessageType::Keepalive);
//                    SendAsync(&keepalive[0], keepalive.size());
                    break;
                }
                case Update: {
                    auto updateMessage = parseBgpUpdateMessage(payloadMessageBytes);
                    std::stringstream message;
                    message << "Received BGP UPDATE message: " << updateMessage.DebugOutput();
                    logging::DEBUG(message.str());
                    fsm_->HandleEvent(BgpUpdateMessageReceived);
                    break;
                }
                case Notification: {
                    auto notificationMessage = parseBgpNotificationMessage(payloadMessageBytes);
                    std::stringstream message;
                    message << "Received BGP NOTIFICATION message: " << notificationMessage.DebugOutput();
                    logging::DEBUG(message.str());
                    fsm_->HandleEvent(BgpNotificationMessageReceived);
                    break;
                }
                case Keepalive: {
                    std::stringstream message;
                    message << "Keepalive message received with Length > 19. This is not RFC4271-compliant.";
                    logging::ERROR(message.str());
                    break;
                }
                case ReservedMessageType:
                case RouteRefresh:
                default: {
                    std::stringstream message;
                    message << "Unsupported message type " << MessageTypeToString(header.Type);
                    logging::ERROR(message.str());
                    break;
                }
            }
        } else if (header.Length == 19 && header.Type == Keepalive) {
            fsm_->HandleEvent(BgpKeepaliveMessageReceived);
        } else {
            std::stringstream message;
            message << "Malformed BGP message received: ["
                      << std::accumulate(messageBytes.begin() + 1, messageBytes.end(), std::to_string(messageBytes[0]),
                                         [](const std::string &a, int b) { return a + ',' + std::to_string(b); })
                      << "]";
            logging::ERROR(message.str());
        }


        {
            std::stringstream message;
            message << "FSM state: " << BgpSessionStateToString(fsm_->State);
            logging::DEBUG(message.str());
        }
    }

    void onError(int error, const std::string &category, const std::string &message) override {
        std::stringstream messageString;
        messageString << "BgpSession caught an error with code " << error << " and category '" << category << "': "
                  << message;
        logging::ERROR(messageString.str());
    }

private:
    std::shared_ptr<BgpFiniteStateMachine> fsm_;
};

class BgpServer : public CppServer::Asio::TCPServer {
public:
    using TCPServer::TCPServer;

    bool Start() override {
        // TODO: track this via user-defined config file (or interactive configuration)
        fsm_ = std::make_shared<BgpFiniteStateMachine>(BgpFiniteStateMachine{0x0A010166, 0x0A010101, 65002, 65001, 16843009, 0,
                                                       AllowAutomaticStop, {}, 180, 60, {}, {}, {}, {}, nullptr, {}});
        fsm_->Start();
        fsm_->HandleEvent(AutomaticStartWithPassiveTcpEstablishment);
        return TCPServer::Start();
    }

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<TCPServer> &server) override {
        return std::make_shared<BgpTCPSession>(server, fsm_);
    }

    void onError(int error, const std::string &category, const std::string &message) override {
        std::stringstream messageString;
        messageString << "BgpServer caught an error with code " << error << " and category '" << category << "': "
                  << message;
        logging::ERROR(messageString.str());
    }

private:
    std::shared_ptr<BgpFiniteStateMachine> fsm_;
};

int main() {
    logging::INFO("Starting BgpServer on port 179.");
    auto service = std::make_shared<CppServer::Asio::Service>();

    logging::INFO("Starting Asio service...");
    service->Start();
    logging::INFO("Asio service started.");

    auto server = std::make_shared<BgpServer>(service, 179);

    logging::INFO("Starting BgpServer...");
    server->Start();
    logging::INFO("BgpServer started.");


    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
    }

    logging::INFO("Stopping BgpServer...");
    server->Stop();
    logging::INFO("BgpServer stopped.");

    logging::INFO("Stopping Asio service...");
    service->Stop();
    logging::INFO("Asio service stopped.");

    return 0;
}
