//
// Created by zach on 10/17/2020.
//

#ifndef BGP_UTIL_H
#define BGP_UTIL_H

#define _16to8(x) static_cast<uint8_t>((x) >> 0x08 & 0xFF), static_cast<uint8_t>((x) & 0xFF)
#define _32to8(x) static_cast<uint8_t>((x) >> 0x18 & 0xFF), static_cast<uint8_t>((x) >> 0x10 & 0xFF), static_cast<uint8_t>((x) >> 0x08 & 0xFF), static_cast<uint8_t>((x) & 0xFF)
#define _128to8(x) static_cast<uint8_t>((x) & 0xFF), static_cast<uint8_t>((x) >> 0x08 & 0xFF), static_cast<uint8_t>((x) >> 0x10 & 0xFF), static_cast<uint8_t>((x) >> 0x18 & 0xFF), static_cast<uint8_t>((x) >> 0x20 & 0xFF), static_cast<uint8_t>((x) >> 0x28 & 0xFF), static_cast<uint8_t>((x) >> 0x30 & 0xFF), static_cast<uint8_t>((x) >> 0x38 & 0xFF), static_cast<uint8_t>((x) >> 0x40 & 0xFF), static_cast<uint8_t>((x) >> 0x48 & 0xFF), static_cast<uint8_t>((x) >> 0x50 & 0xFF), static_cast<uint8_t>((x) >> 0x58 & 0xFF), static_cast<uint8_t>((x) >> 0x60 & 0xFF), static_cast<uint8_t>((x) >> 0x68 & 0xFF), static_cast<uint8_t>((x) >> 0x70 & 0xFF), static_cast<uint8_t>((x) >> 0x78 & 0xFF)

#define _8to16(x, y) static_cast<uint16_t>((y) | static_cast<uint16_t>(x) << 0x08)
#define _8to32(x, y, z, w) static_cast<uint32_t>((w) | static_cast<uint32_t>(z) << 0x08 | static_cast<uint32_t>(y) << 0x10 | static_cast<uint32_t>(x) << 0x18)

#endif //BGP_UTIL_H
