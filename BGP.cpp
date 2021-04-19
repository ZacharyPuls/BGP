//
// Created by zach on 08/23/2020.
//

#include <numeric>
#include <iomanip>
#include <utility>

#include "BGP.h"
#include "BgpOpenMessage.h"
#include "MessageType.h"
#include "BgpHeader.h"
#include "BgpUpdateMessage.h"
#include "BgpNotificationMessage.h"
#include "FiniteStateMachine.h"

class BgpServer {
public:
    // TODO: support for active mode
    // TODO: handle onDisconnected (FSM AutomaticStop)
    BgpServer() {
        // TODO: select interface to listen on based on user-defined config file (or interactive configuration)
        auto serverAddress = std::make_shared<SocketAddress>("", "179");
        server_ = std::make_shared<ServerSocket>(serverAddress);
    }

    void Start() {
        // TODO: track this via user-defined config file (or interactive configuration)
        fsm_ = std::make_shared<BgpFiniteStateMachine>(
                BgpFiniteStateMachine{0x0A010166, 0x0A010101, 65002, 65001, 16843009, 0,
                                      AllowAutomaticStop, {}, 180, 60, {}, {}, {}, {}, [this](auto bytes) { SendMessageToPeer(bytes); }, {}});
        fsm_->Start();
        fsm_->HandleEvent(AutomaticStartWithPassiveTcpEstablishment);

        socket_ = server_->Accept();

        std::stringstream message;
        message << "BgpServer connected to peer " << socket_->address()->to_string();
        logging::DEBUG(message.str());
        fsm_->HandleEvent(TcpConnectionConfirmed);

        // TODO: [14]
        std::vector<uint8_t> messageBytes;
        while (!(messageBytes = socket_->Receive()).empty()) {
            HandleMessage(messageBytes);
        }
    }

private:
    void SendMessageToPeer(const std::vector<uint8_t>& messageBytes) {
        std::stringstream message;
        message << "Sending bytes to peer [" << std::accumulate(messageBytes.begin() + 1, messageBytes.end(), std::to_string(messageBytes[0]), [](const std::string &a, int b) { return a + ',' + std::to_string(b); }) << "]";
        logging::DEBUG(message.str());
        socket_->Send(messageBytes);
    }

    void HandleMessage(const std::vector<uint8_t>& messageBytes) {
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
//                    std::stringstream message;
//                    message << "Received BGP OPEN message: " << openMessage.DebugOutput();
//                    logging::DEBUG(message.str());
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

    std::shared_ptr<ServerSocket> server_;
    std::shared_ptr<TcpSocket> socket_;
    std::shared_ptr<BgpFiniteStateMachine> fsm_;
};

int main() {
    InitializeSocketSubsystem();

    BgpServer server;
    server.Start();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
    }

    ShutdownSocketSubsystem();

    return 0;
}
