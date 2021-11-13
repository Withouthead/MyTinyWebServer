//
// Created by xiaomaotou31 on 2021/11/10.
//

#ifndef MYTINYSERVER_HTTPCONNECT_H
#define MYTINYSERVER_HTTPCONNECT_H

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include "../utils/HttpBuffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
class HttpConnect {
public:
    HttpConnect();
    HttpConnect(int fd_, const sockaddr_in& addr);
    ~HttpConnect();
    ssize_t Read();
    ssize_t Write();
    void Close();

    void Process();
private:
    int client_sockfd;
    sockaddr_in client_sockaddr;
    int iov_cnt;
    iovec iov[2];
    HttpBuffer read_buffer;
    HttpBuffer write_buffer;
    bool is_et;
    bool is_close;

    HttpResponse http_response;
    HttpRequest http_request;




};


#endif //MYTINYSERVER_HTTPCONNECT_H
