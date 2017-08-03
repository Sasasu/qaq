cmake_minimum_required(VERSION 2.8)

project(qaq)
set(QAQ_VERSION_MAJOR 1)
set(QAQ_VERSION_MINOR 0)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(Boost 1.64 REQUIRED COMPONENTS log)

include_directories(${Boost_INCLUDE_DIR})

include_directories(src/socket5)
add_subdirectory(src/socket5)

add_executable(qaq src/main.cpp src/config.cpp src/qaq_server.cpp src/qaq_server.h)

target_link_libraries(qaq socket5)

target_link_libraries(qaq ${Boost_LIBRARIES})
