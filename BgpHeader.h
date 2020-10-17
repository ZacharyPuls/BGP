//
// Created by zach on 10/17/2020.
//

#ifndef BGP_BGPHEADER_H
#define BGP_BGPHEADER_H

#include <cstdint>
#include <array>
#include <vector>
#include <cassert>
#include "MessageType.h"
#include "Util.h"

struct BgpHeader
{
    std::array<uint8_t, 16> Marker;
    uint16_t Length;
    MessageType Type;
};

std::array<uint8_t, 19> generateBgpHeader(const uint16_t payloadLength, const MessageType messageType)
{
    return
            {
                    // Marker
                    0xFF, 0xFF, 0xFF, 0xFF,
                    0xFF, 0xFF, 0xFF, 0xFF,
                    0xFF, 0xFF, 0xFF, 0xFF,
                    0xFF, 0xFF, 0xFF, 0xFF,
                    // Length
                    _16to8(payloadLength + 19),
                    // Type
                    messageType
            };
}

BgpHeader parseBgpHeader(const std::vector<uint8_t>& messageBytes)
{
    assert(messageBytes.size() == 19);

    std::array<uint8_t, 16> marker;

    std::copy_n(messageBytes.begin(), 16, marker.begin());

    return {
            marker,
            _8to16(messageBytes[16], messageBytes[17]),
            static_cast<MessageType>(messageBytes[18])
    };
}

#endif //BGP_BGPHEADER_H
