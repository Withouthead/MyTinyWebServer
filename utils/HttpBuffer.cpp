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

    read_buffer.append(read_temp_buffer);
    return len;
}

ssize_t HttpBuffer::BufferToWrite(const std::string& s) {
    write_buffer.append(s);
    return s.size();
}

ssize_t HttpBuffer::WriteFd(int fd) {
    ssize_t len = send(fd, write_buffer.data(), write_buffer.size(), 0);

    assert(len >= 0);

    write_buffer.clear(); //清空写缓存
    return len;
}

void HttpBuffer::ClearReadBuffer() {
    read_buffer.clear();
}

void HttpBuffer::ClearWriteBuffer() {
    write_buffer.clear();
}

