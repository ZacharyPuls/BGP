//
// Created by zach on 10/17/2020.
//

#ifndef BGP_PATH_H
#define BGP_PATH_H

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

enum PathAttributeFlagBits : uint8_t {
    WellKnown = 0x00,
    Optional = 0x01,
    NonTransitive = 0x00,
    Transitive = 0x02,
    Complete = 0x00,
    Partial = 0x04,
    OneByteAttribute = 0x00,
    TwoByteAttribute = 0x08
};

enum PathAttributeType : uint8_t {
    /*
     * 0 Reserved
     * 1 ORIGIN [RFC4271]
     * 2 AS_PATH [RFC4271]
     * 3 NEXT_HOP [RFC4271]
     * 4 MULTI_EXIT_DISC [RFC4271]
     * 5 LOCAL_PREF [RFC4271]
     * 6 ATOMIC_AGGREGATE [RFC4271]
     * 7 AGGREGATOR [RFC4271]
     * 8 COMMUNITY [RFC1997]
     * 9 ORIGINATOR_ID [RFC4456]
     * 10 CLUSTER_LIST [RFC4456]
     * 14 MP_REACH_NLRI [RFC4760]
     * 15 MP_UNREACH_NLRI [RFC4760]
     * 16 EXTENDED COMMUNITIES [draft-ramachandra-bgp-ext-communities-00][RFC4360]
     * 17 AS4_PATH [RFC6793]
     * 18 AS4_AGGREGATOR [RFC6793]
     * 22 PMSI_TUNNEL [RFC6514]
     * 23 Tunnel Encapsulation Attribute [RFC5512]
     * 24 Traffic Engineering [RFC5543]
     * 25 IPv6 Address Specific Extended Community [RFC5701]
     * 26 AIGP [RFC7311]
     * 27 PE Distinguisher Labels [RFC6514]
     * 29 BGP-LS Attribute [RFC7752]
     * 32 LARGE_COMMUNITY [RFC8092]
     * 33 BGPsec_Path [RFC8205]
     */

    ReservedAttributeType = 0,
    OriginAttribute = 1,
    // TODO: figure out a better way to handle naming collisions between enums and other structs/typedefs. I really don't like doing things like this (naming the enum member 'AsPathAttribute', and the typedef 'AsPath', or changing the capitalization/spelling slightly)
    AsPathAttribute = 2,
    NextHopAttribute = 3,
    MultiExitDiscriminatorAttribute = 4,
    LocalPrefAttribute = 5,
    AtomicAggregateAttribute = 6,
    AggregatorAttribute = 7,
    CommunityAttribute = 8,
    OriginatorIdAttribute = 9,
    ClusterListAttribute = 10,
    MpReachNlriAttribute = 14,
    MpUnreachNlriAttribute = 15,
    ExtendedCommunitiesAttribute = 16,
    As4PathAttribute = 17,
    As4AggregatorAttribute = 18,
    PMSITunnelAttribute = 22,
    TunnelEncapsulationAttribute = 23,
    TrafficEngineeringAttribute = 24,
    Ipv6AddressSpecificExtendedCommunityAttribute = 25,
    AIGPAttribute = 26,
    PEDistinguisherLabelsAttribute = 27,
    BGPLSAttribute = 29,
    LargeCommunityAttribute = 32,
    BGPsecPathAttribute = 33
};

std::string PathAttributeTypeToString(const PathAttributeType pathAttributeType) {
    switch (pathAttributeType) {
        case ReservedAttributeType:
            return "ReservedAttributeType";
        case OriginAttribute:
            return "OriginAttribute";
        case AsPathAttribute:
            return "AsPathAttribute";
        case NextHopAttribute:
            return "NextHopAttribute";
        case MultiExitDiscriminatorAttribute:
            return "MultiExitDiscriminatorAttribute";
        case LocalPrefAttribute:
            return "LocalPrefAttribute";
        case AtomicAggregateAttribute:
            return "AtomicAggregateAttribute";
        case AggregatorAttribute:
            return "AggregatorAttribute";
        case CommunityAttribute:
            return "CommunityAttribute";
        case OriginatorIdAttribute:
            return "OriginatorIdAttribute";
        case ClusterListAttribute:
            return "ClusterListAttribute";
        case MpReachNlriAttribute:
            return "MpReachNlriAttribute";
        case MpUnreachNlriAttribute:
            return "MpUnreachNlriAttribute";
        case ExtendedCommunitiesAttribute:
            return "ExtendedCommunitiesAttribute";
        case As4PathAttribute:
            return "As4PathAttribute";
        case As4AggregatorAttribute:
            return "As4AggregatorAttribute";
        case PMSITunnelAttribute:
            return "PMSITunnelAttribute";
        case TunnelEncapsulationAttribute:
            return "TunnelEncapsulationAttribute";
        case TrafficEngineeringAttribute:
            return "TrafficEngineeringAttribute";
        case Ipv6AddressSpecificExtendedCommunityAttribute:
            return "Ipv6AddressSpecificExtendedCommunityAttribute";
        case AIGPAttribute:
            return "AIGPAttribute";
        case PEDistinguisherLabelsAttribute:
            return "PEDistinguisherLabelsAttribute";
        case BGPLSAttribute:
            return "BGPLSAttribute";
        case LargeCommunityAttribute:
            return "LargeCommunityAttribute";
        case BGPsecPathAttribute:
            return "BGPsecPathAttribute";
        default:
            return "InvalidPathAttributeType";
    }
}

// Well-known mandatory
enum Origin : uint8_t {
    IGP = 0x00,
    EGP = 0x01,
    Incomplete = 0x02
};

enum AsPathSegmentType : uint8_t {
    ASSet = 0x01,
    ASSequence = 0x02
};

struct AsPathSegment {
    AsPathSegmentType Type;
    uint8_t Length;
    // TODO: [3]
    std::vector<uint16_t> Value;
};

// Well-known mandatory
typedef std::vector<AsPathSegment> AsPath;

// TODO: [9], possibly use std::vector<uint8_t>?
// Well-known mandatory
typedef uint32_t NextHop;

// Optional non-transitive
typedef uint32_t MultiExitDiscriminator;

// Well-known discretionary
typedef uint32_t LocalPref;

// Well-known discretionary
typedef uint8_t AtomicAggregate;

// Optional transitive
struct Aggregator {
    // TODO: [3]
    uint16_t LocalAs;
    // TODO: [9]
    uint32_t RouterId;
};

// TODO: [11]

struct PathAttribute {
    uint8_t Flags;
    PathAttributeType Type;

    // TODO: if Flags & PathAttributeFlagBits::TwoByteAttribute, Value[0] and Value[1] contain the length of the attribute data. Otherwise, Value[0] contains the length.
    std::vector<uint8_t> Value;

    [[nodiscard]] std::string DebugOutput() const {
        std::stringstream output;

        output << "Flags: " << std::to_string(Flags) << std::endl;
        output << "Type: " << PathAttributeTypeToString(Type) << std::endl;
        // TODO: also output Value, will need to add string transformations for all possible attribute type/value pairs.

        return output.str();
    }
};

#endif //BGP_PATH_H
