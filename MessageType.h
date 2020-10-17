//
// Created by zach on 10/17/2020.
//

#ifndef BGP_MESSAGETYPE_H
#define BGP_MESSAGETYPE_H

#include <cstdint>
#include <string>

enum MessageType : uint8_t
{
    /*
     * 0 Reserved
     * 1 OPEN [RFC4271]
     * 2 UPDATE [RFC4271]
     * 3 NOTIFICATION [RFC4271]
     * 4 KEEPALIVE [RFC4271]
     * 5 ROUTE-REFRESH [RFC2918]
     */
    ReservedMessageType = 0,
    Open = 1,
    Update = 2,
    Notification = 3,
    Keepalive = 4,
    RouteRefresh = 5,
};

std::string MessageTypeToString(const MessageType messageType)
{
    switch (messageType)
    {
        case ReservedMessageType:
            return "ReservedMessageType";
        case Open:
            return "Open";
        case Update:
            return "Update";
        case Notification:
            return "Notification";
        case Keepalive:
            return "Keepalive";
        case RouteRefresh:
            return "RouteRefresh";
        default:
            return "InvalidMessageType";
    }
}

#endif //BGP_MESSAGETYPE_H
