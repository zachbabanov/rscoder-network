cmake_minimum_required(VERSION 3.16)
project(rscoder-network)

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

set(RECEIVER_SOURCE receiver_point.cpp auxiliary.hpp)

add_executable(receiver ${RECEIVER_SOURCE})
add_custom_command(TARGET receiver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ../csv ../files)
target_include_directories(receiver PUBLIC rscoder)
target_link_libraries(receiver readline)

add_executable(sender ${SENDER_SOURCE})
add_custom_command(TARGET sender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ../csv ../files)
target_include_directories(sender PUBLIC rscoder)
target_link_libraries(sender readline)
