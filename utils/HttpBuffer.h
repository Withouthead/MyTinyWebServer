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
#include <assert.h>

class HttpBuffer {
public:
    ssize_t ReadFd(int fd);
    ssize_t BufferToWrite(const std::string& s);
    ssize_t WriteFd(int fd);
    void ClearReadBuffer();
    void ClearWriteBuffer();
    unsigned long SearchSubString(const std::string& s);
    std::string GetStringFromReadBuffer(int index); //返回包含index及其以前的string
private:
    std::string read_buffer;
    std::string write_buffer;
    int read_buffer_checked_index;


};


#endif //MYTINYSERVER_HTTPBUFFER_H
