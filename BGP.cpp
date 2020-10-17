//
// Created by zach on 08/23/2020.
//

#include "BGP.h"
#include "BgpOpenMessage.h"
#include "MessageType.h"
#include "BgpHeader.h"
#include "BgpUpdateMessage.h"
#include "BgpNotificationMessage.h"
#include "FiniteStateMachine.h"

enum BgpSessionState {
    Idle,
    Connect,
    Active,
    OpenSent,
    OpenConfirm,
    Established
};

enum SessionAttributeFlagBits : uint16_t {
    AcceptConnectionsUnconfiguredPeers = 0x0001,
    AllowAutomaticStart = 0x0002,
    AllowAutomaticStop = 0x0004,
    CollisionDetectEstablishedState = 0x0008,
    DampPeerOscillations = 0x0010,
    DelayOpen = 0x0020,
    PassiveTcpEstablishment = 0x0040,
    SendNotificationWithoutOpen = 0x0080,
    TrackTcpState = 0x0100
};

enum FsmEventType : uint8_t {
    UnknownFsmEventType,
    // Administrative Events
    ManualStart,
    ManualStop,
    AutomaticStart,
    ManualStartWithPassiveTcpEstablishment,
    AutomaticStartWithPassiveTcpEstablishment,
    AutomaticStartWithDampPeerOscillations,
    AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment,
    AutomaticStop,
    // Timer Events
    ConnectRetryTimerExpires,
    HoldTimerExpires,
    KeepaliveTimerExpires,
    DelayOpenTimerExpires,
    IdleHoldTimerExpires,
    // TCP Connection Events
    TcpConnectionValid,
    TcpConnectionRequestInvalid,
    TcpConnectionRequestAcked,
    TcpConnectionConfirmed,
    TcpConnectionFails,
    // BGP Message Events
    BgpOpenMessageReceived,
    BgpOpenWithDelayOpenTimerRunning,
    BgpHeaderError,
    BgpOpenMessageError,
    BgpOpenCollisionDump,
    BgpNotificationMessageVersionError,
    BgpNotificationMessageReceived,
    BgpKeepaliveMessageReceived,
    BgpUpdateMessageReceived,
    BgpUpdateMessageError
};

std::string FsmEventTypeToString(const FsmEventType eventType) {
    switch (eventType) {
        case UnknownFsmEventType:
            return "UnknownFsmEventType";
        case ManualStart:
            return "ManualStart";
        case ManualStop:
            return "ManualStop";
        case AutomaticStart:
            return "AutomaticStart";
        case ManualStartWithPassiveTcpEstablishment:
            return "ManualStartWithPassiveTcpEstablishment";
        case AutomaticStartWithPassiveTcpEstablishment:
            return "AutomaticStartWithPassiveTcpEstablishment";
        case AutomaticStartWithDampPeerOscillations:
            return "AutomaticStartWithDampPeerOscillations";
        case AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment:
            return "AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment";
        case AutomaticStop:
            return "AutomaticStop";
        case ConnectRetryTimerExpires:
            return "ConnectRetryTimerExpires";
        case HoldTimerExpires:
            return "HoldTimerExpires";
        case KeepaliveTimerExpires:
            return "KeepaliveTimerExpires";
        case DelayOpenTimerExpires:
            return "DelayOpenTimerExpires";
        case IdleHoldTimerExpires:
            return "IdleHoldTimerExpires";
        case TcpConnectionValid:
            return "TcpConnectionValid";
        case TcpConnectionRequestInvalid:
            return "TcpConnectionRequestInvalid";
        case TcpConnectionRequestAcked:
            return "TcpConnectionRequestAcked";
        case TcpConnectionConfirmed:
            return "TcpConnectionConfirmed";
        case TcpConnectionFails:
            return "TcpConnectionFails";
        case BgpOpenMessageReceived:
            return "BgpOpenMessageReceived";
        case BgpOpenWithDelayOpenTimerRunning:
            return "BgpOpenWithDelayOpenTimerRunning";
        case BgpHeaderError:
            return "BgpHeaderError";
        case BgpOpenMessageError:
            return "BgpOpenMessageError";
        case BgpOpenCollisionDump:
            return "BgpOpenCollisionDump";
        case BgpNotificationMessageVersionError:
            return "BgpNotificationMessageVersionError";
        case BgpNotificationMessageReceived:
            return "BgpNotificationMessageReceived";
        case BgpKeepaliveMessageReceived:
            return "BgpKeepaliveMessageReceived";
        case BgpUpdateMessageReceived:
            return "BgpUpdateMessageReceived";
        case BgpUpdateMessageError:
            return "BgpUpdateMessageError";
        default:
            return "InvalidFsmEventType";
    }
}

struct BgpSessionTimer {
    uint16_t InitialValue;
    std::atomic<uint16_t> Value;
    std::thread Thread;
    std::atomic<bool> Active;
    std::function<void(FsmEventType)> ExpiredCallback;
    FsmEventType ExpireEventType;

    BgpSessionTimer(const uint16_t initialValue, std::function<void(FsmEventType)> expiredCallback,
                    const FsmEventType expireEventType)
            : InitialValue(initialValue),
              ExpiredCallback(expiredCallback),
              ExpireEventType(expireEventType) {
        Reset();
    }

    BgpSessionTimer(const BgpSessionTimer &other)
            : InitialValue(other.InitialValue),
              ExpiredCallback(other.ExpiredCallback),
              ExpireEventType(other.ExpireEventType) {
        Value.store(other.Value.load(std::memory_order_acquire), std::memory_order_release);
        Active.store(other.Active.load(std::memory_order_acquire), std::memory_order_release);
    }

    BgpSessionTimer &operator=(const BgpSessionTimer &other) {
        if (this == &other)
            return *this;
        InitialValue = other.InitialValue;
        Value.store(other.Value.load(std::memory_order_acquire), std::memory_order_release);
        Active.store(other.Active.load(std::memory_order_acquire), std::memory_order_release);
        ExpiredCallback = other.ExpiredCallback;
        ExpireEventType = other.ExpireEventType;
        return *this;
    }

    BgpSessionTimer &operator=(BgpSessionTimer &&other) {
        if (this == &other)
            return *this;
        InitialValue = other.InitialValue;
        Value.store(other.Value.load(std::memory_order_acquire), std::memory_order_release);
        Active.store(other.Active.load(std::memory_order_acquire), std::memory_order_release);
        ExpiredCallback = std::move(other.ExpiredCallback);
        ExpireEventType = other.ExpireEventType;
        return *this;
    }

    void Start() {
        if (Active.load(std::memory_order_acquire)) {
            Stop();
        }
        Active.store(true, std::memory_order_release);
        Thread = std::thread([this]() {
            while (Active.load(std::memory_order_acquire)) {
                if (Value.load(std::memory_order_acquire) <= 0) {
                    Active.store(false, std::memory_order_release);
                    ExpiredCallback(ExpireEventType);
                } else {
                    // TODO: Support better/higher precision timers, this loop likely doesn't run once per second.
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    Value.store(Value - 1, std::memory_order_release);
                }
            }
        });
    }

    void Stop() {
        Active.store(false, std::memory_order_release);
        if (Thread.joinable()) {
            Thread.join();
        }
    }

    void Restart() {
        Stop();
        Start();
    }

    void Restart(const uint16_t initialValue) {
        Stop();
        Reset(initialValue);
        Start();
    }

    void Reset() {
        Stop();
        Value.store(InitialValue, std::memory_order_release);
    }

    void Reset(const uint16_t initialValue) {
        Stop();
        Value.store(initialValue, std::memory_order_release);
    }
};


class BgpTCPSession : public CppServer::Asio::TCPSession {
public:
    using TCPSession::TCPSession;

protected:

    void sendMessageToPeer(std::vector<uint8_t> messageBytes) {
        SendAsync(&messageBytes[0], messageBytes.size());
    }

    void onConnected() override {
        sockaddr_in sa;
        auto addr_string = server()->address();
        inet_pton(AF_INET, addr_string.c_str(), &(sa.sin_addr));
        auto remote_addr = socket().remote_endpoint().address().to_v4().to_uint();
        fsm_ = BgpFiniteStateMachine(sa.sin_addr.s_addr, remote_addr, 65002, 65001, 16843009, 0, AllowAutomaticStop, {},
                                     {}, {}, {}, {}, {}, {}, [=](auto bytes) { SendAsync(bytes); }, {});
        BgpOpenMessage openMessage = {
                0x04,
                65002,
                180,
                16843009
        };

        // openMessage.Capabilities.emplace_back(BgpCapability{
        // 	MPBGP, 4, {0x01 /*IPv4 AFI*/, 0x00 /*Reserved*/, 0x01 /*Unicast SAFI*/}
        // });
        //
        // openMessage.Capabilities.emplace_back(BgpCapability{RouteRefreshCapability, 0});
        //
        // openMessage.Capabilities.emplace_back(BgpCapability{EnhancedRouteRefresh, 0});
        //
        // openMessage.Capabilities.emplace_back(BgpCapability{FourByteAsn, 4, {_32to8(65002)}});


        auto messageHeader = generateBgpHeader(openMessage.GetLength(), MessageType::Open);
        std::vector<uint8_t> messageBytes(messageHeader.begin(), messageHeader.end());
        auto messageBody = flattenBgpOpenMessage(openMessage);
        messageBytes.insert(messageBytes.end(), messageBody.begin(), messageBody.end());

        SendAsync(&messageBytes[0], messageBytes.size());
    }

    void onDisconnected() override {
        std::cout << "BgpSession with ID " << id() << " was disconnected." << std::endl;
        fsm_.HandleEvent(AutomaticStop);
    }

    void onReceived(const void *buffer, size_t size) override {
        auto *const bytesBuffer = (uint8_t *) buffer;
        std::vector<uint8_t> messageBytes(bytesBuffer, bytesBuffer + size);
        const std::vector<uint8_t> headerMessageBytes(messageBytes.begin(), messageBytes.begin() + 19);

        const auto header = parseBgpHeader(headerMessageBytes);

        std::cout << "Received BGP message header with Type " << MessageTypeToString(header.Type) << " and Length "
                  << header.Length << std::endl;

        if (header.Length > 19) {
            const std::vector<uint8_t> payloadMessageBytes(messageBytes.begin() + 19, messageBytes.end());
            switch (header.Type) {
                case Open: {
                    auto openMessage = parseBgpOpenMessage(payloadMessageBytes);
                    std::cout << "Received BGP OPEN message: " << std::endl;
                    std::cout << openMessage.DebugOutput();
                    auto keepalive = generateBgpHeader(0, MessageType::Keepalive);
                    SendAsync(&keepalive[0], keepalive.size());
                    break;
                }
                case Update: {
                    auto updateMessage = parseBgpUpdateMessage(payloadMessageBytes);
                    std::cout << "Received BGP UPDATE message: " << std::endl;
                    std::cout << updateMessage.DebugOutput();
                    break;
                }
                case Notification: {
                    auto notificationMessage = parseBgpNotificationMessage(payloadMessageBytes);
                    std::cout << "Received BGP NOTIFICATION message: " << std::endl;
                    std::cout << notificationMessage.DebugOutput();
                    break;
                }
                case Keepalive:
                    std::cerr << "Keepalive message received with Length > 19. This is not RFC4271-compliant."
                              << std::endl;
                    break;
                case ReservedMessageType:
                case RouteRefresh:
                default:
                    std::cerr << "Unsupported message type " << MessageTypeToString(header.Type) << std::endl;
                    break;
            }
        }
    }

    void onError(int error, const std::string &category, const std::string &message) override {
        std::cout << "BgpSession caught an error with code " << error << " and category '" << category << "': "
                  << message << std::endl;
    }

private:
    BgpFiniteStateMachine fsm_;
};

class BgpServer : public CppServer::Asio::TCPServer {
public:
    using TCPServer::TCPServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<TCPServer> &server) override {
        return std::make_shared<BgpTCPSession>(server);
    }

    void onError(int error, const std::string &category, const std::string &message) override {
        std::cout << "BgpServer caught an error with code " << error << " and category '" << category << "': "
                  << message << std::endl;
    }

private:

};

int main() {
    std::cout << "Starting BgpServer on port 179." << std::endl;

    auto service = std::make_shared<CppServer::Asio::Service>();

    std::cout << "Starting Asio service...";
    service->Start();
    std::cout << "Done." << std::endl;

    auto server = std::make_shared<BgpServer>(service, 179);

    std::cout << "Starting BgpServer...";
    server->Start();
    std::cout << "Done." << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            break;
        }
    }

    std::cout << "Stopping BgpServer...";
    server->Stop();
    std::cout << "Done." << std::endl;

    std::cout << "Stopping Asio service...";
    service->Stop();
    std::cout << "Done." << std::endl;

    return 0;
}
