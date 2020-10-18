//
// Created by zach on 10/17/2020.
//

#ifndef BGP_ROUTE_H
#define BGP_ROUTE_H

#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "Util.h"

typedef struct ROUTE {
    uint8_t Length;
    uint32_t Prefix;
    // TODO: [9]
    // std::vector<uint8_t> Prefix;

    [[nodiscard]] std::string DebugOutput() const {
        std::stringstream output;
        output << "Length: " << std::to_string(Length) << std::endl;

        const std::vector<uint8_t> prefixDottedDecimal = {_32to8(Prefix)};
        output << "Prefix: " << std::to_string(prefixDottedDecimal[0]) << "." << std::to_string(prefixDottedDecimal[1])
               << "." << std::to_string(prefixDottedDecimal[2]) << "." << std::to_string(prefixDottedDecimal[3]) <<
               std::endl;

        return output.str();
    }
} Route, NLRI;

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

#endif //BGP_ROUTE_H
