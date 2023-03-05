cmake_minimum_required(VERSION 3.24)
project(Hackdis)

set(CMAKE_CXX_STANDARD 20)

add_executable(hackdis-server server/main.cpp common/logger/Logger.cpp)

add_executable(hackdis-client client/main.cpp common/logger/Logger.cpp)
