//
// Created by xiaomaotou31 on 2021/11/13.
//


#include "../HttpUtils/HttpRequest.h"
#include "../HttpUtils/HttpConnect.h"
#include "../HttpUtils/HttpResponse.h"
#include <iostream>
#include <sys/epoll.h>
#include "../webserver/WebServer.h"
using namespace std;

int main(int argc, char *argv[]) {
    int port = 8082;
    WebServer webServer(port, 3, 3000000, true, 3, "./root", "xiaomaotou31", "gao751128", "localhost", "webserver", 3306);
    webServer.StartServer();
    return 0;
}