#pragma once

// #include <cstdio>
// #include <cstdarg>
// #include <gmp.h>
#include <array>
#include <vector>
#include <iostream>
#include <cassert>
#include <functional>
#include <sstream>

#include <server/asio/tcp_server.h>

// TODO: [1] Verify support for RFC 6286 (ASNs do not need to be unique)
// TODO: [2] Support for RFC 6608 (Extended FSM error subcodes)
// TODO: [3] Support for RFC 6793 (Support for 4-byte ASNs)
// TODO: [4] Support for RFC 7606 (Improved UPDATE message error-handling)
// TODO: [5] Support for RFC 7607 (Support for AS0)
// TODO: [6] Support for RFC 7705 (ASN migration mechanisms)
// TODO: [7] Support for RFC 8212 (eBGP default export reject)
// TODO: [8] Support for RFC 8654 (Extend max message size for all message types except OPEN and KEEPALIVE, from 4096 bytes to 65535 bytes)
// TODO: [9] Support for BGPv6, possibly with libgmp, maybe even just with an std::vector<uint8_t>
// TODO: [10] Support for RFC 5065 (Confederations)
// TODO: [11] Support for the rest of the possible path attribute types, reference the IANA registry
// TODO: [12] Evaluate the pros/cons of foregoing using std::vector<uint8_t>, and switching over to raw pointers (uint8_t*, void*, et al). This will require empirical evidence being gathered, via performance/memory tests, including full/multiple table edge cases
// TODO: [13] If a BGP UPDATE message is received with the same prefix in the WithdrawnRoutes and NLRI fields, ignore the prefix in WithdrawnRoutes
