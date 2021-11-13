//
// Created by xiaomaotou31 on 2021/11/10.
//

#ifndef MYTINYSERVER_HTTPBUFFER_H
#define MYTINYSERVER_HTTPBUFFER_H

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>

class HttpBuffer {
public:
    ssize_t ReadFd(int fd);
    ssize_t WriteFd(int fd);
    unsigned long SearchSubString(const std::string& s);
    std::string GetStringFromReadBuffer(size_t index); //返回包含index及其以前的string
    void ClearBuffer();
    size_t size();

    ssize_t Append(const std::string &s);

private:
    std::string buff;

};


#endif //MYTINYSERVER_HTTPBUFFER_H
