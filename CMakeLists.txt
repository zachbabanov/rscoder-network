cmake_minimum_required(VERSION 3.16)
project(untitled)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

if(MSVC)
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SENDER_SOURCE sender_point.cpp auxiliary.hpp command.hpp)

set(RECIEVER_SOURCE reciever_point.cpp auxiliary.hpp)

add_executable(reciever ${RECIEVER_SOURCE})
target_include_directories(reciever PUBLIC rscoder)
target_link_libraries(reciever readline)

add_executable(sender ${SENDER_SOURCE})
target_include_directories(sender PUBLIC rscoder)
target_link_libraries(sender readline)
