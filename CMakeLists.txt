cmake_minimum_required(VERSION 3.17)
project(mytinyserver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
include_directories(/usr/include/mysql)
add_executable(mytinyserver HttpConnect.cpp HttpConnect.h utils/HttpBuffer.cpp utils/HttpBuffer.h HttpUtils/HttpConnect.cpp HttpUtils/HttpConnect.h HttpUtils/HttpRequest.cpp HttpUtils/HttpRequest.h HttpUtils/HttpResponse.cpp HttpUtils/HttpResponse.h test/test.cpp utils/ThreadPool.h utils/HeapTimer.cpp utils/HeapTimer.h webserver/WebServer.cpp webserver/WebServer.h webserver/Epoller.cpp webserver/Epoller.h utils/ServerLog.cpp utils/ServerLog.h utils/BlockQueue.h utils/MySqlPool.cpp utils/MySqlPool.h utils/MySqlPoolRAII.h)
target_link_libraries(mytinyserver Threads::Threads)

target_link_libraries(mytinyserver mysqlclient)
target_link_libraries(mytinyserver nlohmann_json::nlohmann_json)