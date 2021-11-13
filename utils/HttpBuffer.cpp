//
// Created by xiaomaotou31 on 2021/11/10.
//

#include "HttpBuffer.h"
const int READBUFFSIZE = 65535;
ssize_t HttpBuffer::ReadFd(int fd) {
    char read_temp_buffer[READBUFFSIZE];
    memset(read_temp_buffer, 0, sizeof(read_temp_buffer));
    ssize_t len = recv(fd, read_temp_buffer, sizeof(read_temp_buffer), 0);

    assert(len >= 0);

    buff.append(read_temp_buffer);
    return len;
}

ssize_t HttpBuffer::Append(const std::string& s) {
    buff.append(s);
    return s.size();
}

ssize_t HttpBuffer::WriteFd(int fd) {
    ssize_t len = send(fd, buff.data(), buff.size(), 0);

    assert(len >= 0);

    buff.clear(); //清空写缓存
    return len;
}

void HttpBuffer::ClearBuffer() {
    buff.clear();
}

unsigned long HttpBuffer::SearchSubString(const std::string &s) {
    return buff.find(s);
}

std::string HttpBuffer::GetStringFromReadBuffer(size_t index) {//如果找不到是返回string::nop
    std::string sub_string = buff.substr(index + 1);
    buff.erase(0, index + 1);
    return sub_string;
}

size_t HttpBuffer::size() {
    return buff.size();
}

