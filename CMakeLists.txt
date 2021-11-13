cmake_minimum_required(VERSION 3.17)
project(mytinyserver)

set(CMAKE_CXX_STANDARD 14)

add_executable(mytinyserver HttpConnect.cpp HttpConnect.h utils/HttpBuffer.cpp utils/HttpBuffer.h HttpUtils/HttpConnect.cpp HttpUtils/HttpConnect.h HttpUtils/HttpRequest.cpp HttpUtils/HttpRequest.h HttpUtils/HttpResponse.cpp HttpUtils/HttpResponse.h test/test.cpp)