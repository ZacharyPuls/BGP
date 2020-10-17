//
// Created by zach on 10/17/2020.
//

#ifndef BGP_BGPOPENMESSAGE_H
#define BGP_BGPOPENMESSAGE_H

struct BgpOpenMessage
{
    const uint8_t Version = 0x04;
    uint16_t Asn;
    uint16_t HoldTime;
    uint32_t Identifier;
    std::vector<BgpCapability> Capabilities;

    [[nodiscard]] uint16_t GetLength() const
    {
        return static_cast<uint16_t>(10 + GetCapabilitiesLength());
    }

    [[nodiscard]] uint8_t GetCapabilitiesLength() const
    {
        uint8_t capabilitiesLength = 0;

        for (const auto& capability : Capabilities)
        {
            capabilitiesLength += capability.Length + 2;
        }

        return capabilitiesLength;
    }

    [[nodiscard]] std::string DebugOutput() const
    {
        std::stringstream output;
        output << "BGP OPEN, Version " << std::to_string(Version) << std::endl;
        output << "ASN: " << std::to_string(Asn) << std::endl;
        output << "HoldTime: " << std::to_string(HoldTime) << std::endl;
        std::array<uint8_t, 4> identifierDottedDecimal = { _32to8(Identifier) };
        output << "Identifier: " << std::to_string(identifierDottedDecimal[0]) << "." << std::to_string(identifierDottedDecimal[1]) << "." << std::to_string(identifierDottedDecimal[2]) << "." << std::to_string(identifierDottedDecimal[3]) << std::endl;
        output << "Capabilities: " << std::endl;

        for (const auto& capability : Capabilities)
        {
            output << capability.DebugOutput();
        }

        return output.str();
    }
};

std::vector<uint8_t> flattenBgpOpenMessage(const BgpOpenMessage message)
{
    std::vector<uint8_t> openMessage = {
            // Version
            message.Version,
            // My ASN
            _16to8(message.Asn),
            // Hold Time
            _16to8(message.HoldTime),
            // BGP Identifier
            _32to8(message.Identifier),
            // Capabilities Length
            message.GetCapabilitiesLength()
    };

    auto capabilities = flattenBgpCapabilities(message.Capabilities);
    openMessage.insert(openMessage.end(), capabilities.begin(), capabilities.end());

    return openMessage;
}

BgpOpenMessage parseBgpOpenMessage(const std::vector<uint8_t>& messageBytes)
{
    assert(messageBytes.size() >= 9);

    BgpOpenMessage message = {
            messageBytes[0],
            _8to16(messageBytes[1], messageBytes[2]),
            _8to16(messageBytes[3], messageBytes[4]),
            _8to32(messageBytes[5], messageBytes[6], messageBytes[7], messageBytes[8]),
            (messageBytes.size() > 9 ? parseBgpCapabilities(std::vector<uint8_t>(messageBytes.begin() + 9, messageBytes.end())) : std::vector<BgpCapability>{})
    };

    return message;
}

#endif //BGP_BGPOPENMESSAGE_H
