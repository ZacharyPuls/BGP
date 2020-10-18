//
// Created by zach on 10/17/2020.
//

#ifndef BGP_BGPUPDATEMESSAGE_H
#define BGP_BGPUPDATEMESSAGE_H

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <array>
#include "Util.h"
#include "Route.h"
#include "Path.h"

struct BgpUpdateMessage {
    uint16_t WithdrawnRoutesLength;
    std::vector<Route> WithdrawnRoutes;
    uint16_t PathAttributesLength;
    std::vector<PathAttribute> PathAttributes;
    std::vector<NLRI> NLRI;

    [[nodiscard]] std::string DebugOutput() const {
        std::stringstream output;
        output << "BGP UPDATE" << std::endl;
        output << "WithdrawnRoutesLength: " << std::to_string(WithdrawnRoutesLength) << std::endl;

        for (const auto &route : WithdrawnRoutes) {
            output << route.DebugOutput();
        }

        output << "PathAttributesLength: " << std::to_string(PathAttributesLength) << std::endl;

        for (const auto &pathAttribute : PathAttributes) {
            output << pathAttribute.DebugOutput();
        }

        output << "NLRI: " << std::endl;

        for (const auto &nlri : NLRI) {
            output << nlri.DebugOutput();
        }

        return output.str();
    }
};

std::vector<uint8_t> flattenBgpUpdateMessage(const BgpUpdateMessage message) {
    std::vector<uint8_t> updateMessage = {
            _16to8(message.WithdrawnRoutesLength)
    };

    for (const auto &withdrawnRoute : message.WithdrawnRoutes) {
        updateMessage.emplace_back(withdrawnRoute.Length);
        uint8_t prefixBytes[4] = {_32to8(withdrawnRoute.Prefix)};
        updateMessage.emplace_back(prefixBytes[0]);
        updateMessage.emplace_back(prefixBytes[1]);
        updateMessage.emplace_back(prefixBytes[2]);
        updateMessage.emplace_back(prefixBytes[3]);
    }

    uint8_t pathAttributeBytes[2] = {_16to8(message.PathAttributesLength)};
    updateMessage.emplace_back(pathAttributeBytes[0]);
    updateMessage.emplace_back(pathAttributeBytes[1]);

    for (const auto &nlriEntry : message.NLRI) {
        updateMessage.emplace_back(nlriEntry.Length);
        std::array<uint8_t, 4> nlriEntryBytes = {_32to8(nlriEntry.Prefix)};
        updateMessage.emplace_back(nlriEntryBytes[0]);
        updateMessage.emplace_back(nlriEntryBytes[1]);
        updateMessage.emplace_back(nlriEntryBytes[2]);
        updateMessage.emplace_back(nlriEntryBytes[3]);
    }

    auto header = generateBgpHeader(updateMessage.size(), Update);
    updateMessage.insert(updateMessage.begin(), header.begin(), header.end());

    return updateMessage;
}

BgpUpdateMessage parseBgpUpdateMessage(const std::vector<uint8_t> &messageBytes) {
    assert(messageBytes.size() >= 4);
    BgpUpdateMessage message;

    message.WithdrawnRoutesLength = _8to16(messageBytes[0], messageBytes[1]);

    size_t i = 2;

    while (i < 2 + message.WithdrawnRoutesLength) {
        message.WithdrawnRoutes.emplace_back(Route{
                messageBytes[i], _8to32(messageBytes[i + 1], messageBytes[i + 2], messageBytes[i + 3],
                                        messageBytes[i + 4])
        });
        // TODO: [9]

        i += 5;
    }

    message.PathAttributesLength = _8to16(messageBytes[i], messageBytes[i + 1]);

    i += 2;
    auto currentIndex = i;

    while (i < currentIndex + message.PathAttributesLength) {
        PathAttribute attribute = {messageBytes[i], static_cast<PathAttributeType>(messageBytes[i + 1]), {}};

        // if Flags & PathAttributeFlagBits::TwoByteAttribute, Value[0] and Value[1] contain the length of the attribute data. Otherwise, Value[0] contains the length.
        const uint16_t valueLength = attribute.Flags & TwoByteAttribute ? _8to16(messageBytes[i], messageBytes[++i])
                                                                        : static_cast<uint16_t>(messageBytes[i]);
        ++i;

        assert(messageBytes.size() >= i + valueLength);

        attribute.Value = std::vector<uint8_t>(messageBytes.begin() + i, messageBytes.begin() + i + valueLength);

        message.PathAttributes.emplace_back(attribute);

        i += valueLength;
    }

    const auto nlriLength = messageBytes.size() - (i + 1);

    if (nlriLength > 0) {
        currentIndex = i;
        while (i < currentIndex + nlriLength) {
            message.NLRI.emplace_back(NLRI{
                    messageBytes[i], _8to32(messageBytes[i + 1], messageBytes[i + 2], messageBytes[i + 3],
                                            messageBytes[i + 4])
            });
            // TODO: [9]
            // TODO: [13]
            i += 5;
        }
    }

    return message;
}

#endif //BGP_BGPUPDATEMESSAGE_H
