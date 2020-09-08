
#include "BGP.h"

#define _16to8(x) static_cast<uint8_t>((x) >> 0x08 & 0xFF), static_cast<uint8_t>((x) & 0xFF)
#define _32to8(x) static_cast<uint8_t>((x) >> 0x18 & 0xFF), static_cast<uint8_t>((x) >> 0x10 & 0xFF), static_cast<uint8_t>((x) >> 0x08 & 0xFF), static_cast<uint8_t>((x) & 0xFF)
#define _128to8(x) static_cast<uint8_t>((x) & 0xFF), static_cast<uint8_t>((x) >> 0x08 & 0xFF), static_cast<uint8_t>((x) >> 0x10 & 0xFF), static_cast<uint8_t>((x) >> 0x18 & 0xFF), static_cast<uint8_t>((x) >> 0x20 & 0xFF), static_cast<uint8_t>((x) >> 0x28 & 0xFF), static_cast<uint8_t>((x) >> 0x30 & 0xFF), static_cast<uint8_t>((x) >> 0x38 & 0xFF), static_cast<uint8_t>((x) >> 0x40 & 0xFF), static_cast<uint8_t>((x) >> 0x48 & 0xFF), static_cast<uint8_t>((x) >> 0x50 & 0xFF), static_cast<uint8_t>((x) >> 0x58 & 0xFF), static_cast<uint8_t>((x) >> 0x60 & 0xFF), static_cast<uint8_t>((x) >> 0x68 & 0xFF), static_cast<uint8_t>((x) >> 0x70 & 0xFF), static_cast<uint8_t>((x) >> 0x78 & 0xFF)

#define _8to16(x, y) static_cast<uint16_t>((y) | static_cast<uint16_t>(x) << 0x08)
#define _8to32(x, y, z, w) static_cast<uint32_t>((w) | static_cast<uint32_t>(z) << 0x08 | static_cast<uint32_t>(y) << 0x10 | static_cast<uint32_t>(x) << 0x18)

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
		_16to8(payloadLength),
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

struct BgpOpenMessage
{
	const uint8_t Version = 0x04;
	uint16_t Asn;
	uint16_t HoldTime;
	uint32_t Identifier;
	std::vector<BgpCapability> Capabilities;

	[[nodiscard]] uint16_t GetLength() const
	{
		uint16_t capabilitiesLength = 0;

		for (const auto& capability : Capabilities)
		{
			capabilitiesLength += static_cast<uint16_t>(capability.Length) + 2;
		}
		
		return 9 + capabilitiesLength;
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

struct Route
{
	uint8_t Length;
	uint32_t Prefix;
	// TODO: [9]
	// std::vector<uint8_t> Prefix;
	
	[[nodiscard]] std::string DebugOutput() const
	{
		std::stringstream output;
		output << "Length: " << std::to_string(Length) << std::endl;

		const std::vector<uint8_t> prefixDottedDecimal = { _32to8(Prefix) };
		output << "Prefix: " << std::to_string(prefixDottedDecimal[0]) << "." << std::to_string(prefixDottedDecimal[1])
			<< "." << std::to_string(prefixDottedDecimal[2]) << "." << std::to_string(prefixDottedDecimal[3]) <<
			std::endl;

		return output.str();
	}
};

/* TODO: [9]
 *Route generateIPv4Route(const uint8_t length, const uint32_t prefix)
{
	return {
		length,
		{
			_32to8(prefix)
		}
	};
}*/

/* TODO: [9] use libgmp to get arbitrary precision integers
 *Route generateIPv6Route(const uint8_t length, const unsigned __int128 prefix)
{
	return {
		length,
		{
			_128to8(prefix)
		}
	};
}*/

enum PathAttributeFlagBits : uint8_t
{
	WellKnown = 0x00,
	Optional = 0x01,
	NonTransitive = 0x00,
	Transitive = 0x02,
	Complete = 0x00,
	Partial = 0x04,
	OneByteAttribute = 0x00,
	TwoByteAttribute = 0x08
};

enum PathAttributeType : uint8_t
{
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

std::string PathAttributeTypeToString(const PathAttributeType pathAttributeType)
{
	switch (pathAttributeType)
	{
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
enum Origin : uint8_t
{
	IGP = 0x00,
	EGP = 0x01,
	Incomplete = 0x02
};

enum AsPathSegmentType : uint8_t
{
	ASSet = 0x01,
	ASSequence = 0x02
};

struct AsPathSegment
{
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
struct Aggregator
{
	// TODO: [3]
	uint16_t LocalAs;
	// TODO: [9]
	uint32_t RouterId;
};

// TODO: [11]

struct PathAttribute
{
	uint8_t Flags;
	PathAttributeType Type;

	// TODO: if Flags & PathAttributeFlagBits::TwoByteAttribute, Value[0] and Value[1] contain the length of the attribute data. Otherwise, Value[0] contains the length.
	std::vector<uint8_t> Value;
	
	[[nodiscard]] std::string DebugOutput() const
	{
		std::stringstream output;

		output << "Flags: " << std::to_string(Flags) << std::endl;
		output << "Type: " << PathAttributeTypeToString(Type) << std::endl;
		// TODO: also output Value, will need to add string transformations for all possible attribute type/value pairs.

		return output.str();
	}
};

typedef Route NLRI;

struct BgpUpdateMessage
{
	uint16_t WithdrawnRoutesLength;
	std::vector<Route> WithdrawnRoutes;
	uint16_t PathAttributesLength;
	std::vector<PathAttribute> PathAttributes;
	std::vector<NLRI> NLRI;
	
	[[nodiscard]] std::string DebugOutput() const
	{
		std::stringstream output;
		output << "BGP UPDATE" << std::endl;
		output << "WithdrawnRoutesLength: " << std::to_string(WithdrawnRoutesLength) << std::endl;

		for (const auto& route : WithdrawnRoutes)
		{
			output << route.DebugOutput();
		}
		
		output << "PathAttributesLength: " << std::to_string(PathAttributesLength) << std::endl;

		for (const auto& pathAttribute : PathAttributes)
		{
			output << pathAttribute.DebugOutput();
		}

		output << "NLRI: " << std::endl;
		
		for (const auto& nlri : NLRI)
		{
			output << nlri.DebugOutput();
		}

		return output.str();
	}
};

std::vector<uint8_t> flattenBgpUpdateMessage(const BgpUpdateMessage message)
{
	std::vector<uint8_t> updateMessage = {
		_16to8(message.WithdrawnRoutesLength)
	};

	for (const auto& withdrawnRoute : message.WithdrawnRoutes)
	{
		updateMessage.emplace_back(withdrawnRoute.Length);
		uint8_t prefixBytes[4] = { _32to8(withdrawnRoute.Prefix) };
		updateMessage.emplace_back(prefixBytes[0]);
		updateMessage.emplace_back(prefixBytes[1]);
		updateMessage.emplace_back(prefixBytes[2]);
		updateMessage.emplace_back(prefixBytes[3]);
	}

	uint8_t pathAttributeBytes[2] = { _16to8(message.PathAttributesLength) };
	updateMessage.emplace_back(pathAttributeBytes[0]);
	updateMessage.emplace_back(pathAttributeBytes[1]);

	for (const auto& nlriEntry : message.NLRI)
	{
		updateMessage.emplace_back(nlriEntry.Length);
		std::array<uint8_t, 4> nlriEntryBytes = { _32to8(nlriEntry.Prefix) };
		updateMessage.emplace_back(nlriEntryBytes[0]);
		updateMessage.emplace_back(nlriEntryBytes[1]);
		updateMessage.emplace_back(nlriEntryBytes[2]);
		updateMessage.emplace_back(nlriEntryBytes[3]);
	}

	return updateMessage;
}

BgpUpdateMessage parseBgpUpdateMessage(const std::vector<uint8_t>& messageBytes)
{
	assert(messageBytes.size() >= 4);
	BgpUpdateMessage message;

	message.WithdrawnRoutesLength = _8to16(messageBytes[0], messageBytes[1]);

	size_t i = 2;

	while (i < 2 + message.WithdrawnRoutesLength)
	{
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

	while (i < currentIndex + message.PathAttributesLength)
	{
		PathAttribute attribute = {messageBytes[i], static_cast<PathAttributeType>(messageBytes[i + 1]), {}};
		
		// if Flags & PathAttributeFlagBits::TwoByteAttribute, Value[0] and Value[1] contain the length of the attribute data. Otherwise, Value[0] contains the length.
		const uint16_t valueLength = attribute.Flags & TwoByteAttribute ? _8to16(messageBytes[i], messageBytes[++i]) : static_cast<uint16_t>(messageBytes[i]);
		++i;
		
		assert(messageBytes.size() >= i + valueLength);
		
		attribute.Value = std::vector<uint8_t>(messageBytes.begin() + i, messageBytes.begin() + i + valueLength);
	
		message.PathAttributes.emplace_back(attribute);
		
		i += valueLength;
	}

	const auto nlriLength = messageBytes.size() - (i + 1);

	if (nlriLength > 0)
	{
		currentIndex = i;
		while (i < currentIndex + nlriLength)
		{
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
	BgpError Error;
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

class BgpSession : public CppServer::Asio::TCPSession
{
public:
	using TCPSession::TCPSession;

protected:
	void onConnected() override
	{
		BgpOpenMessage openMessage = {
			0x04,
			65002,
			180,
			16843009
		};

		openMessage.Capabilities.emplace_back(BgpCapability{
			MPBGP, 4, {0x01 /*IPv4 AFI*/, 0x00 /*Reserved*/, 0x01 /*Unicast SAFI*/}
		});

		openMessage.Capabilities.emplace_back(BgpCapability{RouteRefreshCapability, 0});

		openMessage.Capabilities.emplace_back(BgpCapability{EnhancedRouteRefresh, 0});

		openMessage.Capabilities.emplace_back(BgpCapability{FourByteAsn, 4, {_32to8(65002)}});

		
		auto messageHeader = generateBgpHeader(openMessage.GetLength(), MessageType::Open);
		std::vector<uint8_t> messageBytes(messageHeader.begin(), messageHeader.end());
		auto messageBody = flattenBgpOpenMessage(openMessage);
		messageBytes.insert(messageBytes.end(), messageBody.begin(), messageBody.end());

		SendAsync(&messageBytes[0], messageBytes.size());
	}

	void onDisconnected() override
	{
		std::cout << "BgpSession with ID " << id() << " was disconnected." << std::endl;
	}
	
	void onReceived(const void* buffer, size_t size) override
	{
		auto* const bytesBuffer = (uint8_t*)buffer;
		std::vector<uint8_t> messageBytes(bytesBuffer, bytesBuffer + size);
		const std::vector<uint8_t> headerMessageBytes(messageBytes.begin(), messageBytes.begin() + 19);

		const auto header = parseBgpHeader(headerMessageBytes);
		
		std::cout << "Received BGP message header with Type " << MessageTypeToString(header.Type) << " and Length " << header.Length << std::endl;

		if (header.Length > 19) {
			const std::vector<uint8_t> payloadMessageBytes(messageBytes.begin() + 19, messageBytes.end());
			switch (header.Type)
			{
			case Open:
			{
				auto openMessage = parseBgpOpenMessage(payloadMessageBytes);
				std::cout << "Received BGP OPEN message: " << std::endl;
				std::cout << openMessage.DebugOutput();
				break;
			}
			case Update:
			{
				auto updateMessage = parseBgpUpdateMessage(payloadMessageBytes);
				std::cout << "Received BGP UPDATE message: " << std::endl;
				std::cout << updateMessage.DebugOutput();
				break;
			}
			case Notification:
				{
				auto notificationMessage = parseBgpNotificationMessage(payloadMessageBytes);
				std::cout << "Received BGP NOTIFICATION message: " << std::endl;
				std::cout << notificationMessage.DebugOutput();
				break;
				}
			case Keepalive:
				std::cerr << "Keepalive message received with Length > 19. This is not RFC4271-compliant." << std::endl;
				break;
			case ReservedMessageType:
			case RouteRefresh:
			default:
				std::cerr << "Unsupported message type " << MessageTypeToString(header.Type) << std::endl;
				break;
			}
		}
	}

	void onError(int error, const std::string& category, const std::string& message) override
	{
		std::cout << "BgpSession caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
	}
};

class BgpServer : public CppServer::Asio::TCPServer
{
public:
	using TCPServer::TCPServer;

protected:
	std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<TCPServer>& server) override
	{
		return std::make_shared<BgpSession>(server);
	}

	void onError(int error, const std::string& category, const std::string& message) override
	{
		std::cout << "BgpServer caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
	}
};

int main()
{
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
	while (std::getline(std::cin, line))
	{
		if (line.empty())
		{
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
