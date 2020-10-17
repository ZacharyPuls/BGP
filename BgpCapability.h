//
// Created by zach on 10/17/2020.
//

#ifndef BGP_BGPCAPABILITY_H
#define BGP_BGPCAPABILITY_H

#include <cstdint>
#include <string>

enum CapabilityCode : uint8_t
{
    /*
     * 0 	Reserved [RFC5492]
     * 1 	Multiprotocol Extensions for BGP-4 [RFC2858]
     * 2 	Route Refresh Capability for BGP-4 [RFC2918]
     * 3 	Outbound Route Filtering Capability [RFC5291]
     * 4 	Multiple routes to a destination capability (deprecated) [RFC8277]
     * 5 	Extended Next Hop Encoding [RFC5549]
     * 6 	BGP Extended Message [RFC8654]
     * 7 	BGPsec Capability [RFC8205]
     * 8 	Multiple Labels Capability [RFC8277]
     * 9 	BGP Role [draft-ietf-idr-bgp-open-policy]
     * 64 	Graceful Restart Capability [RFC4724]
     * 65 	Support for 4-octet AS number Capability [RFC6793]
     * 67 	Support for Dynamic Capability [draft-ietf-idr-dynamic-cap]
     * 68 	Multisession BGP Capability [draft-ietf-idr-bgp-multisession]
     * 69 	ADD-PATH Capability [RFC7911]
     * 70 	Enhanced Route Refresh Capability [RFC7313]
     * 71 	Long-Lived Graceful Restart (LLGR) Capability [draft-uttaro-idr-bgp-persistence]
     * 72 	Routing Policy Distribution [draft-ietf-idr-rpd-04]
     * 73 	FQDN Capability [draft-walton-bgp-hostname-capability]
     */

    ReservedCapabilityCode = 0,
    MPBGP = 1,
    RouteRefreshCapability = 2,
    OutboundRouteFiltering = 3,
    MultipleRoutesToDestination = 4,
    ExtendedNextHopEncoding = 5,
    ExtendedMessage = 6,
    BGPSec = 7,
    MultipleLabels = 8,
    BGPRole = 9,
    GracefulRestart = 64,
    FourByteAsn = 65,
    Dynamic = 67,
    Multisession = 68,
    AddPath = 69,
    EnhancedRouteRefresh = 70,
    LLGR = 71,
    RoutingPolicyDistribution = 72,
    FQDN = 73
};

std::string CapabilityCodeToString(const CapabilityCode capabilityCode)
{
    switch (capabilityCode)
    {
        case ReservedCapabilityCode:
            return "ReservedCapabilityCode";
        case MPBGP:
            return "MPBGP";
        case RouteRefreshCapability:
            return "RouteRefreshCapability";
        case OutboundRouteFiltering:
            return "OutboundRouteFiltering";
        case MultipleRoutesToDestination:
            return "MultipleRoutesToDestination";
        case ExtendedNextHopEncoding:
            return "ExtendedNextHopEncoding";
        case ExtendedMessage:
            return "ExtendedMessage";
        case BGPSec:
            return "BGPSec";
        case MultipleLabels:
            return "MultipleLabels";
        case BGPRole:
            return "BGPRole";
        case GracefulRestart:
            return "GracefulRestart";
        case FourByteAsn:
            return "FourByteAsn";
        case Dynamic:
            return "Dynamic";
        case Multisession:
            return "Multisession";
        case AddPath:
            return "AddPath";
        case EnhancedRouteRefresh:
            return "EnhancedRouteRefresh";
        case LLGR:
            return "LLGR";
        case RoutingPolicyDistribution:
            return "RoutingPolicyDistribution";
        case FQDN:
            return "FQDN";
        default:
            return "InvalidCapabilityCode";
    }
}

struct BgpCapability
{
    CapabilityCode Code;
    uint8_t Length;
    std::vector<uint8_t> Value;

    std::string DebugOutput() const
    {
        std::stringstream output;
        output << "Capability: " << CapabilityCodeToString(Code) << std::endl;
        output << "Length: " << std::to_string(Length) << std::endl;
        output << "Value: [";

        for (uint8_t i = 0; i < Length; ++i)
        {
            output << std::to_string(Value[i]) << ",";
        }

        output << "]" << std::endl;
        return output.str();
    }
};

std::vector<uint8_t> flattenBgpCapabilities(const std::vector<BgpCapability>& capabilities)
{
    std::vector<uint8_t> flattenedCapabilities(capabilities.size() * 3);

    for (const auto& capability : capabilities)
    {
        flattenedCapabilities.emplace_back(static_cast<uint8_t>(capability.Code));
        flattenedCapabilities.emplace_back(capability.Length);
        flattenedCapabilities.insert(flattenedCapabilities.end(), capability.Value.begin(), capability.Value.end());
    }

    return flattenedCapabilities;
}

std::vector<BgpCapability> parseBgpCapabilities(const std::vector<uint8_t>& capabilitiesBytes)
{
    assert(capabilitiesBytes.size() >= 2);
    std::vector<BgpCapability> capabilities;
    const auto capabilitiesLength = capabilitiesBytes[0];

    assert(capabilitiesLength == static_cast<uint8_t>(capabilitiesBytes.size() - 1));

    size_t i = 1;

    do
    {
        const auto optionalParameterType = capabilitiesBytes[i];
        assert(optionalParameterType == 0x02);
        const auto optionalParameterLength = capabilitiesBytes[i + 1];
        assert(i + 2 + optionalParameterLength <= capabilitiesBytes.size());
        auto dataStart = capabilitiesBytes.begin() + i + 4;
        const auto capabilityLength = capabilitiesBytes[i + 3];
        BgpCapability capability = {
                static_cast<CapabilityCode>(capabilitiesBytes[i + 2]),
                capabilityLength,
                std::vector<uint8_t>(dataStart, dataStart + capabilityLength)
        };

        capabilities.emplace_back(capability);

        i += 2 + optionalParameterLength;
    } while (i < capabilitiesBytes.size());

    return capabilities;
}

#endif //BGP_BGPCAPABILITY_H
