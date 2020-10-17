# CMakeList.txt : CMake project for BGP, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("BGP")

# set(CMAKE_WIN32_SDK "C:\\Program Files (x86)\\Windows Kits\\10")
# add_subdirectory(extern/CppServer)
# include_directories(extern/CppServer/include)

# Add source to this project's executable.
add_executable (BGP BGP.cpp BGP.h FiniteStateMachine.h MessageType.h BgpHeader.h Util.h BgpCapability.h BgpOpenMessage.h Route.h Path.h BgpUpdateMessage.h BgpError.h BgpNotificationMessage.h)

set_property(TARGET BGP PROPERTY CXX_STANDARD 20)
set_property(TARGET BGP PROPERTY CXX_STANDARD_REQUIRED true)

include("extern/cppserver.cmake")
target_link_libraries(BGP cppserver)

#set(CPPSERVER_DIR "C:\\Users\\zach\\OneDrive\\Programming\\Libraries\\CppServer")

#include_directories(${CPPSERVER_DIR}/include ${CPPSERVER_DIR}/modules/asio/asio/include ${CPPSERVER_DIR}/modules/OpenSSL/VS/include ${CPPSERVER_DIR}/modules/CppCommon/include)
#target_link_libraries(BGP ${CPPSERVER_DIR}/bin/cppserver.lib)

# ExternalProject_Add(libgmp PREFIX ${PROJECT_SOURCE_DIR}/lib/gmp SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/gmp-src)
# add_subdirectory(${PROJECT_SOURCE_DIR}/lib/gmp)
# add_library(BGP STATIC gmp)