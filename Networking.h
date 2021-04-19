//
// Created by zach on 2021-04-17.
//

#ifndef BGP_NETWORKING_H
#define BGP_NETWORKING_H

// TODO: portability :)
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ServerSocket.h"
#include "Log.h"

// TODO: make this return a bool, fail fast when WSAStartup fails.
void InitializeSocketSubsystem() {
    WSADATA wsaData;
    auto retval = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (retval != 0) {
        std::stringstream errorMessage;
        errorMessage << "WSAStartup failed with error code " << retval;
        logging::ERROR(errorMessage.str());
    }
}

void ShutdownSocketSubsystem() {
    WSACleanup();
}

#endif //BGP_NETWORKING_H
