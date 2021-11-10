cmake_minimum_required(VERSION 3.17)
project(mytinyserver)

set(CMAKE_CXX_STANDARD 14)

add_executable(mytinyserver main.cpp HttpConnect.cpp HttpConnect.h utils/HttpBuffer.cpp utils/HttpBuffer.h)