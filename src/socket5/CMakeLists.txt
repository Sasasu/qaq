cmake_minimum_required(VERSION 2.8)

project(socket5)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.65 REQUIRED COMPONENTS thread)

include_directories(${Boost_INCLUDE_DIR})

add_library(socket5 socket_server.cpp address.cpp)

target_link_libraries(socket5 ${Boost_LIBRARIES})

if (UNIX)
    target_link_libraries(socket5 pthread)
endif ()
