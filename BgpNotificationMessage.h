//
// Created by zach on 10/17/2020.
//

#ifndef BGP_BGPNOTIFICATIONMESSAGE_H
#define BGP_BGPNOTIFICATIONMESSAGE_H

#include <vector>
#include <cassert>
#include "BgpError.h"

struct BgpNotificationMessage
{

    // TODO: Error-handling for marker not being all 1s, set ErrorSubcode to MessageHeaderErrorSubcode::ConnectionNotSynchronized
    // TODO: for the following situations, set ErrorSubcode to MessageHeaderErrorSubcode::BadMessageLength, and Data to the erroneous Length field:
    //		- if the Length field of the message header is less than 19 or greater than 4096
    //		- if the Length field of an OPEN message is less than the minimum length of the OPEN message
    //		- if the Length field of an UPDATE message is less than the minimum length of the UPDATE message
    //		- if the Length field of a KEEPALIVE message is not equal to 19
    //		- if the Length field of a NOTIFICATION message is less than the minimum length of the NOTIFICATION message
    // TODO: If the Type field in the header is unrecognized, set ErrorSubcode to MessageHeaderErrorSubcode::BadMessageType
    // TODO: If the BgpOpenMessage fails with OpenMessageErrorSubcode::UnsupportedVersionNumber, check the header to find the BGP version the peer tried, and fall back to that version
    // TODO: Handle all error scenarios detailed in RFC 4271 Section 6
    BgpError Error{};
    std::vector<uint8_t> Data;

    [[nodiscard]] std::string DebugOutput() const
    {
        std::stringstream output;

        output << "Error: " << std::endl;
        output << Error.DebugOutput();
        // TODO: Also handle/output Data field.

        return output.str();
    }
};

std::vector<uint8_t> flattenBgpNotificationMessage(const BgpNotificationMessage message)
{
    std::vector<uint8_t> notificationMessage = { message.Error.Code, message.Error.Subcode };
    notificationMessage.insert(notificationMessage.end(), message.Data.begin(), message.Data.end());
    return notificationMessage;
}

BgpNotificationMessage parseBgpNotificationMessage(const std::vector<uint8_t>& messageBytes)
{
    assert(messageBytes.size() >= 2);
    BgpNotificationMessage message;

    message.Error = BgpError{ messageBytes[0], messageBytes[1] };
    if (messageBytes.size() > 2) {
        message.Data = std::vector<uint8_t>(messageBytes.begin() + 2, messageBytes.end());
    }

    return message;
}

#endif //BGP_BGPNOTIFICATIONMESSAGE_H
