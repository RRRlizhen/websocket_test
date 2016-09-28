cmake_minimum_required(VERSION 2.8)
project(web_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(./client/json)
include_directories(./depends/include/websocketpp)


set(SOURCE_FILES
        server/main.cpp
        client/main.cpp
        client/cclient.cpp
        client/cclient.h)
add_executable(web_server ${SOURCE_FILES})