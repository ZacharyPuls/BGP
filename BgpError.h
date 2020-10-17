//
// Created by zach on 10/17/2020.
//

#ifndef BGP_BGPERROR_H
#define BGP_BGPERROR_H

#include <cstdint>
#include <string>
#include <sstream>

enum NotificationErrorCode : uint8_t
{
    /*
     * 0 Reserved
     * 1 Message Header Error [RFC4271]
     * 2 OPEN Message Error [RFC4271]
     * 3 UPDATE Message Error [RFC4271]
     * 4 Hold Timer Expired [RFC4271]
     * 5 Finite State Machine Error [RFC4271]
     * 6 Cease [RFC4271]
     * 7 ROUTE-REFRESH Message Error [RFC7313]
     */
    ReservedNotificationErrorCode = 0,
    MessageHeaderError = 1,
    OpenMessageError = 2,
    UpdateMessageError = 3,
    HoldTimerExpired = 4,
    FSMError = 5,
    CeaseError = 6,
    RouteRefreshMessageError = 7
};

enum MessageHeaderErrorSubcode : uint8_t
{
    /*
     * 0 Unspecific [RFC Errata 4493]
     * 1 Connection Not Synchronized [RFC4271]
     * 2 Bad Message Length [RFC4271]
     * 3 Bad Message Type [RFC4271]
     */
    UnspecificMessageHeaderError = 0,
    ConnectionNotSynchronized = 1,
    BadMessageLength = 2,
    BadMessageType = 3
};

std::string MessageHeaderErrorSubcodeToString(const MessageHeaderErrorSubcode subcode)
{
    switch (subcode)
    {
        case UnspecificMessageHeaderError:
            return "UnspecificMessageHeaderError";
        case ConnectionNotSynchronized:
            return "ConnectionNotSynchronized";
        case BadMessageLength:
            return "BadMessageLength";
        case BadMessageType:
            return "BadMessageType";
        default:
            return "InvalidMessageHeaderErrorSubcode";
    }
}

enum OpenMessageErrorSubcode : uint8_t
{
    /*
     * 0 Unspecific [RFC Errata 4493]
     * 1 Unsupported Version Number [RFC4271]
     * 2 Bad Peer AS [RFC4271]
     * 3 Bad BGP Identifier [RFC4271]
     * 4 Unsupported Optional Parameter [RFC4271]
     * 6 Unacceptable Hold Time [RFC4271]
     * 7 Unsupported Capability [RFC5492]
     * 8 Role Mismatch [draft-ietf-idr-bgp-open-policy]
     */
    UnspecificOpenMessageError = 0,
    UnsupportedVersionNumber = 1,
    BadPeerAs = 2,
    BadBgpIdentifier = 3,
    UnsupportedOptionalParameter = 4,
    UnacceptableHoldTime = 6,
    UnsupportedCapability = 7,
    RoleMismatch = 8
};

std::string OpenMessageErrorSubcodeToString(const OpenMessageErrorSubcode subcode)
{
    switch (subcode)
    {
        case UnspecificOpenMessageError:
            return "UnspecificOpenMessageError";
        case UnsupportedVersionNumber:
            return "UnsupportedVersionNumber";
        case BadPeerAs:
            return "BadPeerAs";
        case BadBgpIdentifier:
            return "BadBgpIdentifier";
        case UnsupportedOptionalParameter:
            return "UnsupportedOptionalParameter";
        case UnacceptableHoldTime:
            return "UnacceptableHoldTime";
        case UnsupportedCapability:
            return "UnsupportedCapability";
        case RoleMismatch:
            return "RoleMismatch";
        default:
            return "InvalidOpenMessageErrorSubcode";
    }
}

enum UpdateMessageErrorSubcode : uint8_t
{
    /*
     * 0 Unspecific [RFC Errata 4493]
     * 1 Malformed Attribute List [RFC4271]
     * 2 Unrecognized Well-known Attribute [RFC4271]
     * 3 Missing Well-known Attribute [RFC4271]
     * 4 Attribute Flags Error [RFC4271]
     * 5 Attribute Length Error [RFC4271]
     * 6 Invalid ORIGIN Attribute [RFC4271]
     * 8 Invalid NEXT_HOP Attribute [RFC4271]
     * 9 Optional Attribute Error [RFC4271]
     * 10 Invalid Network Field [RFC4271]
     * 11 Malformed AS_PATH [RFC4271]
     */
    UnspecificUpdateMessageError = 0,
    MalformedAttributeList = 1,
    UnrecognizedWellKnownAttribute = 2,
    MissingWellKnownAttribute = 3,
    AttributeFlagsError = 4,
    AttributeLengthError = 5,
    InvalidOriginAttribute = 6,
    InvalidNextHopAttribute = 8,
    OptionalAttributeError = 9,
    InvalidNetworkField = 10,
    MalformedAsPath = 11
};

std::string UpdateMessageErrorSubcodeToString(const UpdateMessageErrorSubcode subcode)
{
    switch (subcode)
    {
        case UnspecificUpdateMessageError:
            return "UnspecificUpdateMessageError";
        case MalformedAttributeList:
            return "MalformedAttributeList";
        case UnrecognizedWellKnownAttribute:
            return "UnrecognizedWellKnownAttribute";
        case MissingWellKnownAttribute:
            return "MissingWellKnownAttribute";
        case AttributeFlagsError:
            return "AttributeFlagsError";
        case AttributeLengthError:
            return "AttributeLengthError";
        case InvalidOriginAttribute:
            return "InvalidOriginAttribute";
        case InvalidNextHopAttribute:
            return "InvalidNextHopAttribute";
        case OptionalAttributeError:
            return "OptionalAttributeError";
        case InvalidNetworkField:
            return "InvalidNetworkField";
        case MalformedAsPath:
            return "MalformedAsPath";
        default:
            return "InvalidUpdateMessageErrorSubcode";
    }
}

enum FSMErrorSubcode : uint8_t
{
    /*
     * 0 Unspecified Error [RFC6608]
     * 1 Receive Unexpected Message in OpenSent State [RFC6608]
     * 2 Receive Unexpected Message in OpenConfirm State [RFC6608]
     * 3 Receive Unexpected Message in Established State [RFC6608]
     */
    UnspecifiedFSMError = 0,
    ReceivedUnexpectedMessageInOpenSentState = 1,
    ReceivedUnexpectedMessageInOpenConfirmState = 2,
    ReceivedUnexpectedMessageInEstablishedState = 3
};

std::string FSMErrorSubcodeToString(const FSMErrorSubcode subcode)
{
    switch (subcode)
    {
        case UnspecifiedFSMError:
            return "UnspecifiedFSMError";
        case ReceivedUnexpectedMessageInOpenSentState:
            return "ReceivedUnexpectedMessageInOpenSentState";
        case ReceivedUnexpectedMessageInOpenConfirmState:
            return "ReceivedUnexpectedMessageInOpenConfirmState";
        case ReceivedUnexpectedMessageInEstablishedState:
            return "ReceivedUnexpectedMessageInEstablishedState";
        default:
            return "InvalidFSMErrorSubcode";
    }
}
enum CeaseErrorSubcode : uint8_t
{
    /*
     * 0 Reserved
     * 1 Maximum Number of Prefixes Reached [RFC4486]
     * 2 Administrative Shutdown [RFC4486][RFC8203]
     * 3 Peer De-configured [RFC4486]
     * 4 Administrative Reset [RFC4486][RFC8203]
     * 5 Connection Rejected [RFC4486]
     * 6 Other Configuration Change [RFC4486]
     * 7 Connection Collision Resolution [RFC4486]
     * 8 Out of Resources [RFC4486]
     * 9 Hard Reset [RFC8538]
     */
    CeaseErrorReserved = 0,
    MaximumNumberOfPrefixesReached = 1,
    AdministrativeShutdown = 2,
    PeerDeconfigured = 3,
    AdministrativeReset = 4,
    ConnectionRejected = 5,
    OtherConfigurationChange = 6,
    ConnectionCollisionResolution = 7,
    OutOfResources = 8,
    HardReset = 9
};

std::string CeaseErrorSubcodeToString(const CeaseErrorSubcode subcode)
{
    switch (subcode)
    {
        case CeaseErrorReserved:
            return "CeaseErrorReserved";
        case MaximumNumberOfPrefixesReached:
            return "MaximumNumberOfPrefixesReached";
        case AdministrativeShutdown:
            return "AdministrativeShutdown";
        case PeerDeconfigured:
            return "PeerDeconfigured";
        case AdministrativeReset:
            return "AdministrativeReset";
        case ConnectionRejected:
            return "ConnectionRejected";
        case OtherConfigurationChange:
            return "OtherConfigurationChange";
        case ConnectionCollisionResolution:
            return "ConnectionCollisionResolution";
        case OutOfResources:
            return "OutOfResources";
        case HardReset:
            return "HardReset";
        default:
            return "InvalidCeaseErrorSubcode";
    }
}

enum RouteRefreshMessageErrorSubcode : uint8_t
{
    /*
     * 0 Reserved [RFC7313]
     * 1 Invalid Message Length [RFC7313]
     */
    RouteRefreshMessageErrorReserved = 0,
    InvalidMessageLength = 1
};

std::string RouteRefreshMessageErrorSubcodeToString(const RouteRefreshMessageErrorSubcode subcode)
{
    switch (subcode)
    {
        case RouteRefreshMessageErrorReserved:
            return "RouteRefreshMessageErrorReserved";
        case InvalidMessageLength:
            return "InvalidMessageLength";
        default:
            return "InvalidRouteRefreshMessageErrorSubcode";
    }
}

struct BgpError
{
    uint8_t Code;
    uint8_t Subcode;

    [[nodiscard]] std::string DebugOutput() const
    {
        std::stringstream output;

        switch (Code)
        {
            case ReservedNotificationErrorCode:
                output << "ReservedNotificationErrorCode" << std::endl;
                break;
            case MessageHeaderError:
                output << "MessageHeaderError: " << MessageHeaderErrorSubcodeToString(static_cast<MessageHeaderErrorSubcode>(Subcode)) << std::endl;
                break;
            case OpenMessageError:
                output << "OpenMessageError: " << OpenMessageErrorSubcodeToString(static_cast<OpenMessageErrorSubcode>(Subcode)) << std::endl;
                break;
            case UpdateMessageError:
                output << "UpdateMessageError: " << UpdateMessageErrorSubcodeToString(static_cast<UpdateMessageErrorSubcode>(Subcode)) << std::endl;
                break;
            case HoldTimerExpired:
                output << "HoldTimerExpired" << std::endl;
                break;
            case FSMError:
                output << "FSMError: " << FSMErrorSubcodeToString(static_cast<FSMErrorSubcode>(Subcode)) << std::endl;
                break;
            case CeaseError:
                output << "CeaseError: " << CeaseErrorSubcodeToString(static_cast<CeaseErrorSubcode>(Subcode)) << std::endl;
                break;
            case RouteRefreshMessageError:
                output << "RouteRefreshMessageError: " << RouteRefreshMessageErrorSubcodeToString(static_cast<RouteRefreshMessageErrorSubcode>(Subcode)) << std::endl;
                break;
            default:
                output << "Invalid BgpError Code " << std::to_string(Code) << std::endl;
                break;
        }

        return output.str();
    }
};

#endif //BGP_BGPERROR_H
