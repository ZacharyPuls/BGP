# CMakeList.txt : CMake project for BGP, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("BGP")

# Add source to this project's executable.
add_executable (BGP "BGP.cpp" "BGP.h")

set_property(TARGET BGP PROPERTY CXX_STANDARD 20)
set_property(TARGET BGP PROPERTY CXX_STANDARD_REQUIRED true)

set(Boost_INCLUDE_DIR extern/boost)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.74 REQUIRED COMPONENTS system thread regex)
message(STATUS "Boost version: ${Boost_VERSION}")


# ExternalProject_Add(libgmp PREFIX ${PROJECT_SOURCE_DIR}/lib/gmp SOURCE_DIR ${PROJECT_SOURCE_DIR}/lib/gmp-src)
# add_subdirectory(${PROJECT_SOURCE_DIR}/lib/gmp)
# add_library(BGP STATIC gmp)