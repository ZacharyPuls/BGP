//
// Created by zach on 10/17/2020.
//

#ifndef BGP_FINITESTATEMACHINE_H
#define BGP_FINITESTATEMACHINE_H

#include <cstdint>
#include <utility>
#include <functional>
#include "Log.h"

enum BgpSessionState {
    Idle,
    Connect,
    Active,
    OpenSent,
    OpenConfirm,
    Established
};

std::string BgpSessionStateToString(const BgpSessionState sessionState) {
    switch (sessionState) {
        case Idle:
            return "Idle";
        case Connect:
            return "Connect";
        case Active:
            return "Active";
        case OpenSent:
            return "OpenSent";
        case OpenConfirm:
            return "OpenConfirm";
        case Established:
            return "Established";
        default:
            return "InvalidBgpSessionState";
    }
}

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

struct BgpFiniteStateMachine;

static void HandleTimerEvent(const std::shared_ptr<BgpFiniteStateMachine>& fsm, FsmEventType eventType);

struct BgpSessionTimer {
    uint16_t InitialValue{};
    std::atomic<uint16_t> Value;
    std::thread Thread;
    std::atomic<bool> Active;
    std::shared_ptr<BgpFiniteStateMachine> Parent;
    FsmEventType ExpireEventType;

    BgpSessionTimer() = default;

    BgpSessionTimer(const uint16_t initialValue, std::shared_ptr<BgpFiniteStateMachine> parent,
                    const FsmEventType expireEventType)
            : InitialValue(initialValue),
              Parent(std::move(parent)),
              ExpireEventType(expireEventType) {
        Reset();
    }

    BgpSessionTimer &operator=(const BgpSessionTimer &other) {
        if (this == &other)
            return *this;
        InitialValue = other.InitialValue;
        Value.store(other.Value.load(std::memory_order_acquire), std::memory_order_release);
        Active.store(other.Active.load(std::memory_order_acquire), std::memory_order_release);
        Parent = other.Parent;
        ExpireEventType = other.ExpireEventType;
        return *this;
    }

    void Start() {
        if (Active.load(std::memory_order_acquire)) {
            Stop();
        }
        Active.store(true, std::memory_order_release);
        Thread = std::thread(BgpSessionTimer::TimerThread, this);
    }

    static void TimerThread(BgpSessionTimer* timer) {
        while (timer->Active.load(std::memory_order_acquire)) {
            if (timer->Value.load(std::memory_order_acquire) <= 0) {
                timer->Active.store(false, std::memory_order_release);
                HandleTimerEvent(timer->Parent, timer->ExpireEventType);
            } else {
                // TODO: Support better/higher precision timers, this loop likely doesn't run once per second.
                std::this_thread::sleep_for(std::chrono::seconds(1));
                timer->Value.store(timer->Value - 1, std::memory_order_release);
            }
        }
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

struct BgpFiniteStateMachine {
    // TODO: [9]
    uint32_t LocalIpAddress;
    uint32_t RemoteIpAddress;
    // TODO: [3]
    uint16_t LocalAsn;
    uint16_t RemoteAsn;
    uint32_t LocalRouterId;
    uint32_t RemoteRouterId;

    BgpSessionState State = Idle;
    uint16_t ConnectRetryCounter = 0;
    SessionAttributeFlagBits Attributes;

    // Timer initial values
    uint16_t ConnectRetryTime = 120;
    uint16_t HoldTime = 90;
    uint16_t KeepaliveTime = 90 / 3;
    uint16_t MinASOriginationIntervalTime;
    uint16_t MinRouteAdvertisementIntervalTime;
    uint16_t DelayOpenTime;
    uint16_t IdleHoldTime;

    // Timers
    BgpSessionTimer ConnectRetryTimer;
    BgpSessionTimer HoldTimer;
    BgpSessionTimer KeepaliveTimer;
    BgpSessionTimer MinASOriginationIntervalTimer;
    BgpSessionTimer MinRouteAdvertisementIntervalTimer;
    BgpSessionTimer DelayOpenTimer;
    BgpSessionTimer IdleHoldTimer;

    std::function<void(std::vector<uint8_t>)> SendMessageToPeer = [](auto bytes) { std::stringstream message; message << "Empty std::function Bgp::FiniteStateMachine::SendMessageToPeer called."; logging::ERROR(message.str()); };

    std::vector<BgpCapability> Capabilities;


    BgpFiniteStateMachine(const uint32_t localIpAddress, const uint32_t remoteIpAddress, const uint16_t localAsn,
                          const uint16_t remoteAsn, const uint32_t localRouterId, const uint32_t remoteRouterId,
                          const SessionAttributeFlagBits attributes, const uint16_t connectRetryTime = 120,
                          const uint16_t holdTime = 90, const uint16_t keepaliveTime = 90 / 3,
                          const uint16_t minAsOriginationIntervalTime = 0,
                          const uint16_t minRouteAdvertisementIntervalTime = 0, const uint16_t delayOpenTime = 0,
                          const uint16_t idleHoldTime = 0,
                          std::function<void(std::vector<uint8_t>)> sendMessageToPeer = nullptr,
                          const std::vector<BgpCapability> capabilities = {}) : LocalIpAddress(localIpAddress),
                                                                                RemoteIpAddress(remoteIpAddress),
                                                                                LocalAsn(localAsn),
                                                                                RemoteAsn(remoteAsn),
                                                                                LocalRouterId(localRouterId),
                                                                                RemoteRouterId(remoteRouterId),
                                                                                Attributes(attributes),
                                                                                ConnectRetryTime(connectRetryTime),
                                                                                HoldTime(holdTime),
                                                                                KeepaliveTime(keepaliveTime),
                                                                                MinASOriginationIntervalTime(
                                                                                        minAsOriginationIntervalTime),
                                                                                MinRouteAdvertisementIntervalTime(
                                                                                        minRouteAdvertisementIntervalTime),
                                                                                DelayOpenTime(delayOpenTime),
                                                                                IdleHoldTime(idleHoldTime),
                                                                                ConnectRetryTimer(connectRetryTime,
                                                                                                  std::shared_ptr<BgpFiniteStateMachine>(
                                                                                                          this),
                                                                                                  ConnectRetryTimerExpires),
                                                                                HoldTimer(holdTime,
                                                                                          std::shared_ptr<BgpFiniteStateMachine>(
                                                                                                  this),
                                                                                          HoldTimerExpires),
                                                                                KeepaliveTimer(keepaliveTime,
                                                                                               std::shared_ptr<BgpFiniteStateMachine>(
                                                                                                       this),
                                                                                               KeepaliveTimerExpires),
                                                                                MinASOriginationIntervalTimer(
                                                                                        minAsOriginationIntervalTime,
                                                                                        std::shared_ptr<BgpFiniteStateMachine>(
                                                                                                this),
                                                                                        UnknownFsmEventType),
                                                                                MinRouteAdvertisementIntervalTimer(
                                                                                        minRouteAdvertisementIntervalTime,
                                                                                        std::shared_ptr<BgpFiniteStateMachine>(
                                                                                                this),
                                                                                        UnknownFsmEventType),
                                                                                DelayOpenTimer(delayOpenTime,
                                                                                               std::shared_ptr<BgpFiniteStateMachine>(
                                                                                                       this),
                                                                                               DelayOpenTimerExpires),
                                                                                IdleHoldTimer(idleHoldTime,
                                                                                              std::shared_ptr<BgpFiniteStateMachine>(
                                                                                                      this),
                                                                                              IdleHoldTimerExpires),
                                                                                SendMessageToPeer(
                                                                                        std::move(sendMessageToPeer)),
                                                                                Capabilities(capabilities) {
    }

    BgpFiniteStateMachine(const BgpFiniteStateMachine &other) {
        this->LocalIpAddress = other.LocalIpAddress;
        this->RemoteIpAddress = other.RemoteIpAddress;
        this->LocalAsn = other.LocalAsn;
        this->RemoteAsn = other.RemoteAsn;
        this->LocalRouterId = other.LocalRouterId;
        this->RemoteRouterId = other.RemoteRouterId;

        this->State = other.State;
        this->ConnectRetryCounter = other.ConnectRetryCounter;
        this->Attributes = other.Attributes;

        this->ConnectRetryTime = other.ConnectRetryTime;
        this->HoldTime = other.HoldTime;
        this->KeepaliveTime = other.KeepaliveTime;
        this->MinASOriginationIntervalTime = other.MinASOriginationIntervalTime;
        this->MinRouteAdvertisementIntervalTime = other.MinRouteAdvertisementIntervalTime;
        this->DelayOpenTime = other.DelayOpenTime;
        this->IdleHoldTime = other.IdleHoldTime;

        this->ConnectRetryTimer = other.ConnectRetryTimer;
        this->HoldTimer = other.HoldTimer;
        this->KeepaliveTimer = other.KeepaliveTimer;
        this->MinASOriginationIntervalTimer = other.MinASOriginationIntervalTimer;
        this->MinRouteAdvertisementIntervalTimer = other.MinRouteAdvertisementIntervalTimer;
        this->DelayOpenTimer = other.DelayOpenTimer;
        this->IdleHoldTimer = other.IdleHoldTimer;

        this->SendMessageToPeer = other.SendMessageToPeer;

        this->Capabilities = other.Capabilities;
    }

    uint16_t ApplyJitter(const uint16_t value) {
        return static_cast<uint16_t>(static_cast<float>(75 + rand() % 100) / 100.0f * static_cast<float>(value));
    }

    void Start() {
        State = Idle;
        ConnectRetryCounter = 0;
        ConnectRetryTimer.InitialValue = ConnectRetryTime;
        ConnectRetryTimer.Reset();
    }

    void HandleEvent(const FsmEventType eventType) {
        std::stringstream message;
        message << "Handling FSM event " << FsmEventTypeToString(eventType) << " in state " << BgpSessionStateToString(State);
        logging::DEBUG(message.str());
        switch (State) {
            case Idle:
                HandleEventInIdleState(eventType);
                break;
            case Connect:
                HandleEventInConnectState(eventType);
                break;
            case Active:
                HandleEventInActiveState(eventType);
                break;
            case OpenSent:
                HandleEventInOpenSentState(eventType);
                break;
            case OpenConfirm:
                HandleEventInOpenConfirmState(eventType);
                break;
            case Established:
                HandleEventInEstablishedState(eventType);
                break;
        }
    }

    void HandleEventInIdleState(const FsmEventType eventType) {
        switch (eventType) {
            case ManualStop:
            case AutomaticStop:
            case ConnectRetryTimerExpires:
            case HoldTimerExpires:
            case KeepaliveTimerExpires:
            case DelayOpenTimerExpires:
                break;
            case ManualStart:
            case AutomaticStart:
                ConnectRetryCounter = 0;
                ConnectRetryTimer.Start();
                // Initiate TCP connection to peer
                // Listen for TCP connection from peer
                State = Connect;
                break;
            case ManualStartWithPassiveTcpEstablishment:
            case AutomaticStartWithPassiveTcpEstablishment:
                ConnectRetryCounter = 0;
                ConnectRetryTimer.Start();
                // Listen for TCP connection from peer
                State = Active;
                break;
            case AutomaticStartWithDampPeerOscillations:
            case AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment:
            case IdleHoldTimerExpires:
                // TODO: implement damp peer oscillation
                break;
            default:
                break;
        }
    }

    void HandleEventInConnectState(const FsmEventType eventType) {
        switch (eventType) {
            case AutomaticStart:
            case ManualStartWithPassiveTcpEstablishment:
            case AutomaticStartWithPassiveTcpEstablishment:
            case AutomaticStartWithDampPeerOscillations:
            case AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment:
                break;
            case ManualStop:
                // Drop the TCP connection
                ConnectRetryCounter = 0;
                ConnectRetryTimer.Stop();
                State = Idle;
                break;
            case ConnectRetryTimerExpires:
                // Drop the TCP connection
                ConnectRetryTimer.Restart();
                DelayOpenTimer.Restart();
                // Initiate TCP connection to peer
                // Continue to listen for TCP connection that may be initiated from peer
                break;
            case DelayOpenTimerExpires:
                // Send OPEN message to peer
                SendMessageToPeer(flattenBgpOpenMessage({0x04, LocalAsn, HoldTime, LocalRouterId, Capabilities}));
                HoldTimer.Restart(UINT16_MAX);
                State = OpenSent;
                break;
            case TcpConnectionValid:
                // Process TCP connection from peer
                break;
            case TcpConnectionRequestInvalid:
                // Reject TCP connection from peer
                break;
            case TcpConnectionRequestAcked:
            case TcpConnectionConfirmed:
                if (Attributes & DelayOpen) {
                    ConnectRetryTimer.Reset(0);
                    DelayOpenTimer.Restart();
                } else {
                    ConnectRetryTimer.Reset(0);
                    // Complete BGP initialization
                    // Send OPEN message to peer
                    SendMessageToPeer(flattenBgpOpenMessage({0x04, LocalAsn, HoldTime, LocalRouterId, Capabilities}));
                    HoldTimer.Restart(UINT16_MAX);
                    State = OpenSent;
                }
            case TcpConnectionFails:
                if (DelayOpenTimer.Active.load(std::memory_order_acquire)) {
                    ConnectRetryTimer.Restart();
                    DelayOpenTimer.Reset(0);
                    // Continue to listen for connection that may be initiated by peer
                    State = Active;
                } else {
                    ConnectRetryTimer.Reset(0);
                    // Drop TCP connection
                    State = Idle;
                }
                break;
            case BgpOpenWithDelayOpenTimerRunning: {
                ConnectRetryTimer.Reset(0);
                // Complete BGP initialization
                DelayOpenTimer.Reset(0);
                // Send OPEN message
                SendMessageToPeer(flattenBgpOpenMessage({0x04, LocalAsn, HoldTime, LocalRouterId, Capabilities}));
                // Send KEEPALIVE message
                std::vector<uint8_t> keepaliveMessageBytes(18);
                auto messageHeader = generateBgpHeader(0, MessageType::Keepalive);
                keepaliveMessageBytes.insert(keepaliveMessageBytes.begin(), messageHeader.begin(), messageHeader.end());
                SendMessageToPeer(keepaliveMessageBytes);
                if (HoldTimer.InitialValue != 0) {
                    KeepaliveTimer.Start();
                    HoldTimer.Restart();
                } else {
                    KeepaliveTimer.Restart();
                    HoldTimer.Reset(0);
                }
                State = OpenConfirm;
                break;
            }
            case BgpHeaderError:
            case BgpOpenMessageError: {
                if (Attributes & SendNotificationWithoutOpen) {
                    // Send NOTIFICATION message
                    BgpNotificationMessage message;
                    message.Error.Code = eventType == BgpHeaderError ? MessageHeaderError : OpenMessageError;
                    message.Error.Subcode =
                            eventType == BgpHeaderError ? UnspecificMessageHeaderError : UnspecificOpenMessageError;
                    SendMessageToPeer(flattenBgpNotificationMessage(message));
                }
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            }
            case BgpNotificationMessageVersionError: {
                if (DelayOpenTimer.Active.load(std::memory_order_acquire)) {
                    ConnectRetryTimer.Reset(0);
                    DelayOpenTimer.Reset(0);
                    // Release all resources
                    // Drop TCP connection
                    State = Idle;
                } else {
                    ConnectRetryTimer.Reset(0);
                    // Release all resources
                    // Drop TCP connection
                    ++ConnectRetryCounter;
                    if (Attributes & DampPeerOscillations) {
                        // TODO: support for damp peer oscillations
                    }
                    State = Idle;
                }
                break;
            }
            default:
                break;
        }
    }

    void HandleEventInActiveState(FsmEventType eventType) {
        switch (eventType) {
            case ManualStart:
            case AutomaticStart:
            case ManualStartWithPassiveTcpEstablishment:
            case AutomaticStartWithPassiveTcpEstablishment:
            case AutomaticStartWithDampPeerOscillations:
            case AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment:
                break;
            case ManualStop:
                if (DelayOpenTimer.Active.load(std::memory_order_acquire) && Attributes & SendNotificationWithoutOpen) {
                    // Send NOTIFICATION with CEASE
                    BgpNotificationMessage message;
                    message.Error.Code = CeaseError;
                    message.Error.Subcode = AdministrativeShutdown;
                    SendMessageToPeer(flattenBgpNotificationMessage(message));
                }
                // Release resources
                DelayOpenTimer.Stop();
                // Drop TCP connection
                ConnectRetryCounter = 0;
                ConnectRetryTimer.Reset(0);
                State = Idle;
                break;
            case ConnectRetryTimerExpires:
                ConnectRetryTimer.Restart();
                // Initiate TCP connection to peer
                // Continue to listen for TCP connection from peer
                State = Connect;
                break;
            case DelayOpenTimerExpires:
                ConnectRetryTimer.Reset(0);
                DelayOpenTimer.Reset(0);
                // Complete BGP initialization
                // Send OPEN message
                SendMessageToPeer(flattenBgpOpenMessage({0x04, LocalAsn, HoldTime, LocalRouterId, Capabilities}));
                HoldTimer.Restart(UINT16_MAX);
                State = OpenSent;
                break;
            case TcpConnectionValid:
                // Process TCP connection from peer
                break;
            case TcpConnectionRequestInvalid:
                // Reject TCP connection from peer
                break;
            case TcpConnectionRequestAcked:
            case TcpConnectionConfirmed:
                logging::TRACE("BgpFiniteStateMachine::ActiveState::TcpConnectionConfirmed");
                if (Attributes & DelayOpen) {
                    ConnectRetryTimer.Reset(0);
                    DelayOpenTimer.Restart();
                } else {
                    ConnectRetryTimer.Reset(0);
                    // Complete BGP initialization
                    // Send OPEN message to peer
                    logging::TRACE("BgpFiniteStateMachine::ActiveState::TcpConnectionConfirmed::SendOPENMessageToPeer");
                    SendMessageToPeer(flattenBgpOpenMessage({0x04, LocalAsn, HoldTime, LocalRouterId, Capabilities}));
                    logging::TRACE("BgpFiniteStateMachine::ActiveState::TcpConnectionConfirmed::SendOPENMessageToPeer::Complete");
                    HoldTimer.Restart(UINT16_MAX);
                    State = OpenSent;
                }
                break;
            case TcpConnectionFails:
                ConnectRetryTimer.Restart();
                DelayOpenTimer.Reset(0);
                // Release all resources
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case BgpOpenWithDelayOpenTimerRunning:
                ConnectRetryTimer.Reset(0);
                // Complete BGP initialization
                DelayOpenTimer.Reset(0);
                // Send OPEN message
                SendMessageToPeer(flattenBgpOpenMessage({0x04, LocalAsn, HoldTime, LocalRouterId, Capabilities}));
                // Send KEEPALIVE message
                SendKeepaliveMessage();
                if (HoldTimer.InitialValue != 0) {
                    KeepaliveTimer.Start();
                    HoldTimer.Restart();
                } else {
                    KeepaliveTimer.Restart();
                    HoldTimer.Reset(0);
                }
                State = OpenConfirm;
                break;
            case BgpHeaderError:
            case BgpOpenMessageError:
                if (Attributes & SendNotificationWithoutOpen) {
                    // Send NOTIFICATION message
                    SendNotificationMessage(OpenMessageError, 0x00);
                }
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case BgpNotificationMessageVersionError:
                if (DelayOpenTimer.Active.load(std::memory_order_acquire)) {
                    ConnectRetryTimer.Reset(0);
                    DelayOpenTimer.Reset(0);
                    // Release all resources
                    // Drop TCP connection
                    State = Idle;
                } else {
                    ConnectRetryTimer.Reset(0);
                    // Release all resources
                    // Drop TCP connection
                    ++ConnectRetryCounter;
                    if (Attributes & DampPeerOscillations) {
                        // TODO: support for damp peer oscillations
                    }
                    State = Idle;
                }
                break;
            case AutomaticStop:
            case HoldTimerExpires:
            case KeepaliveTimerExpires:
            case IdleHoldTimerExpires:
            case BgpOpenMessageReceived:
            case BgpOpenCollisionDump:
            case BgpNotificationMessageReceived:
            case BgpKeepaliveMessageReceived:
            case BgpUpdateMessageReceived:
            case BgpUpdateMessageError:
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            default:
                break;
        }
    }

    void SendNotificationMessage(uint8_t Code, uint8_t Subcode) {
        BgpNotificationMessage message;
        message.Error.Code = Code;
        message.Error.Subcode = Subcode;
        SendMessageToPeer(flattenBgpNotificationMessage(message));
    }

    void SendKeepaliveMessage() {
        std::vector<uint8_t> messageBytes;
        auto headerBytes = generateBgpHeader(0x00, Keepalive);
        messageBytes.insert(messageBytes.end(), headerBytes.begin(), headerBytes.end());
        SendMessageToPeer(messageBytes);
    }

    void HandleEventInOpenSentState(const FsmEventType eventType) {
        switch (eventType) {
            case ManualStart:
            case AutomaticStart:
            case ManualStartWithPassiveTcpEstablishment:
            case AutomaticStartWithPassiveTcpEstablishment:
            case AutomaticStartWithDampPeerOscillations:
            case AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment:
                break;
            case ManualStop: {
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, AdministrativeShutdown);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ConnectRetryCounter = 0;
                State = Idle;
                break;
            }
            case AutomaticStop: {
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, AdministrativeShutdown);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            }
            case HoldTimerExpires:
                // Send NOTIFICATION with Hold Timer Expired
                SendNotificationMessage(HoldTimerExpired, 0x00);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case TcpConnectionValid:
            case TcpConnectionRequestAcked:
            case TcpConnectionConfirmed:
                // A second TCP connection may be in progress. Track this with connection collision processing until an OPEN message is received
            case TcpConnectionRequestInvalid:
                break;
            case TcpConnectionFails:
                // Close BGP connection
                ConnectRetryTimer.Restart();
                // Continue to listen for a connection that may be initiated by the remote peer
                State = Active;
                break;
            case BgpOpenMessageReceived:
                // TODO: check message fields for correctness. Either do that here, and send an event 21 or 22 if there's an error, or do it at a higher level and propagate the event to this handler.
                DelayOpenTimer.Reset(0);
                ConnectRetryTimer.Reset(0);
                // Send KEEPALIVE message
                SendKeepaliveMessage();
                if (HoldTimer.InitialValue > 0) {
                    KeepaliveTimer.Restart();
                    HoldTimer.Restart();
                }
                State = OpenConfirm;
                // TODO: implement collision detection
                break;
            case BgpHeaderError:
            case BgpOpenMessageError:
                // Send NOTIFICATION with error code
                SendNotificationMessage(OpenMessageError, UnspecificOpenMessageError);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case BgpOpenCollisionDump:
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, ConnectionCollisionResolution);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case BgpNotificationMessageVersionError:
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                State = Idle;
                break;
            case ConnectRetryTimerExpires:
            case KeepaliveTimerExpires:
            case DelayOpenTimerExpires:
            case IdleHoldTimerExpires:
            case BgpOpenWithDelayOpenTimerRunning:
            case BgpNotificationMessageReceived:
            case BgpKeepaliveMessageReceived:
            case BgpUpdateMessageReceived:
            case BgpUpdateMessageError:
                // Send NOTIFICATION with FSM error
                SendNotificationMessage(FSMError, ReceivedUnexpectedMessageInOpenSentState);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            default:
                break;
        }
    }

    void HandleEventInOpenConfirmState(FsmEventType eventType) {
        switch (eventType) {
            case ManualStart:
            case AutomaticStart:
            case ManualStartWithPassiveTcpEstablishment:
            case AutomaticStartWithPassiveTcpEstablishment:
            case AutomaticStartWithDampPeerOscillations:
            case AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment:
                break;
            case ManualStop:
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, AdministrativeShutdown);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ConnectRetryCounter = 0;
                State = Idle;
                break;
            case AutomaticStop:
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, AdministrativeShutdown);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case HoldTimerExpires:
                // Send NOTIFICATION with Hold Timer Expired
                SendNotificationMessage(HoldTimerExpired, 0x00);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case KeepaliveTimerExpires:
                // Send KEEPALIVE
                SendKeepaliveMessage();
                KeepaliveTimer.Restart();
                break;
            case TcpConnectionValid:
            case TcpConnectionRequestAcked:
            case TcpConnectionConfirmed:
                // Need to track second TCP connection
                break;
            case TcpConnectionRequestInvalid:
                // Ignore second connection
                break;
            case TcpConnectionFails:
            case BgpNotificationMessageReceived:
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case BgpNotificationMessageVersionError:
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                State = Idle;
                break;
            case BgpOpenMessageReceived:
                if (0) {
                    // TODO: implement collision detection
                    // Send NOTIFICATION with CEASE
                    SendNotificationMessage(CeaseError, ConnectionCollisionResolution);
                    // Release all resources
                    // Drop TCP connection (TCP FIN)
                    ++ConnectRetryCounter;
                    if (Attributes & DampPeerOscillations) {
                        // TODO: support for damp peer oscillations
                    }
                    State = Idle;
                }
                break;
            case BgpHeaderError:
            case BgpOpenMessageError:
                // Send NOTIFICATION message
                SendNotificationMessage(OpenMessageError, 0x00);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case BgpOpenCollisionDump:
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, ConnectionCollisionResolution);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case BgpKeepaliveMessageReceived:
                HoldTimer.Restart();
                State = Established;
                break;
            case ConnectRetryTimerExpires:
            case DelayOpenTimerExpires:
            case IdleHoldTimerExpires:
            case BgpOpenWithDelayOpenTimerRunning:
            case BgpUpdateMessageReceived:
            case BgpUpdateMessageError:
                // Send NOTIFICATION with FSM error
                SendNotificationMessage(FSMError, ReceivedUnexpectedMessageInOpenConfirmState);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
        }
    }

    void HandleEventInEstablishedState(FsmEventType eventType) {
        switch (eventType) {
            case ManualStart:
            case AutomaticStart:
            case ManualStartWithPassiveTcpEstablishment:
            case AutomaticStartWithPassiveTcpEstablishment:
            case AutomaticStartWithDampPeerOscillations:
            case AutomaticStartWithDampPeerOscillationsAndPassiveTcpEstablishment:
                break;
            case ManualStop:
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, AdministrativeShutdown);
                ConnectRetryTimer.Reset(0);
                // Delete all routes associated with this connection
                // Release all resources
                // Drop TCP connection
                ConnectRetryCounter = 0;
                State = Idle;
                break;
            case AutomaticStop:
                // Send NOTIFICATION with CEASE
                SendNotificationMessage(CeaseError, AdministrativeShutdown);
                ConnectRetryTimer.Reset(0);
                // Delete all routes associated with this connection
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case HoldTimerExpires:
                // Send NOTIFICATION with hold timer expired
                SendNotificationMessage(HoldTimerExpired, 0x00);
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case KeepaliveTimerExpires:
                // Send KEEPALIVE
                SendKeepaliveMessage();
                if (HoldTimer.InitialValue > 0) {
                    KeepaliveTimer.Restart();
                }
                break;
            case TcpConnectionValid:
                // Need to track second TCP connection
                break;
            case TcpConnectionRequestInvalid:
                // Ignore second connection
                break;
            case TcpConnectionRequestAcked:
            case TcpConnectionConfirmed:
                // Need to track second TCP connection until OPEN message is sent
                break;
            case BgpOpenMessageReceived:
                if (Attributes & CollisionDetectEstablishedState) {
                    // TODO: propagate a BgpOpenCollisionDump event
                }
                break;
            case BgpOpenCollisionDump:
                // TODO: implement collision detection
                if (0) {
                    // Send NOTIFICATION with CEASE
                    SendNotificationMessage(CeaseError, ConnectionCollisionResolution);
                    ConnectRetryTimer.Reset(0);
                    // Delete all routes associated with this connection
                    // Release all resources
                    // Drop TCP connection
                    ++ConnectRetryCounter;
                    if (Attributes & DampPeerOscillations) {
                        // TODO: support for damp peer oscillations
                    }
                    State = Idle;
                    break;
                }
                break;
            case BgpNotificationMessageVersionError:
            case BgpNotificationMessageReceived:
            case TcpConnectionFails:
                ConnectRetryTimer.Reset(0);
                // Delete all routes associated with this connection
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                State = Idle;
            case BgpKeepaliveMessageReceived:
                if (HoldTimer.InitialValue > 0) {
                    HoldTimer.Restart();
                }
                break;
            case BgpUpdateMessageReceived:
                // Process UPDATE
                // TODO: if an error is encountered, propagate a BgpUpdateMessageError event
                if (HoldTimer.InitialValue > 0) {
                    HoldTimer.Restart();
                }
                break;
            case BgpUpdateMessageError:
                // Send NOTIFICATION with UPDATE error
                SendNotificationMessage(UpdateMessageError, UnspecificUpdateMessageError);
                ConnectRetryTimer.Reset(0);
                // Delete all routes associated with this connection
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
            case ConnectRetryTimerExpires:
            case DelayOpenTimerExpires:
            case IdleHoldTimerExpires:
            case BgpOpenWithDelayOpenTimerRunning:
            case BgpHeaderError:
            case BgpOpenMessageError:
                // Send NOTIFICATION with FSM error
                SendNotificationMessage(FSMError, ReceivedUnexpectedMessageInEstablishedState);
                // Delete all routes associated with this connection
                ConnectRetryTimer.Reset(0);
                // Release all resources
                // Drop TCP connection
                ++ConnectRetryCounter;
                if (Attributes & DampPeerOscillations) {
                    // TODO: support for damp peer oscillations
                }
                State = Idle;
                break;
        }
    }
};

// Wrapping HandleEvent call for [...]TimerExpired event in static method to see if it will fix the timer thread
//  not receiving proper state from FiniteStateMachine when called
static void HandleTimerEvent(const std::shared_ptr<BgpFiniteStateMachine>& fsm, const FsmEventType eventType) {
    std::stringstream message;
    message << "Current FSM state - " << BgpSessionStateToString(fsm->State);
    logging::TRACE(message.str());
    fsm->HandleEvent(eventType);
}

#endif //BGP_FINITESTATEMACHINE_H
