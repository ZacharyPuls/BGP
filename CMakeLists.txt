cmake_minimum_required (VERSION 3.8)

project ("BGP")

add_compile_definitions(LOGGING_LEVEL_ALL)

# Add source to this project's executable.
add_executable (BGP BGP.cpp BGP.h FiniteStateMachine.h MessageType.h BgpHeader.h Util.h BgpCapability.h BgpOpenMessage.h Route.h Path.h BgpUpdateMessage.h BgpError.h BgpNotificationMessage.h Log.h Common.h IPAddress.h SocketAddress.h Socket.h ServerSocket.h Networking.h)

set_property(TARGET BGP PROPERTY CXX_STANDARD 20)
set_property(TARGET BGP PROPERTY CXX_STANDARD_REQUIRED true)

#add_subdirectory("extern/date")

target_link_libraries(BGP)
target_include_directories(BGP PRIVATE "extern/date/include")