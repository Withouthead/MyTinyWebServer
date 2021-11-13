//
// Created by xiaomaotou31 on 2021/11/10.
//

#include "HttpConnect.h"

ssize_t HttpConnect::Read() {
    ssize_t len = 0;
    do {
        len = read_buffer.ReadFd(client_sockfd);
        if (len <= 0)
            break;
    } while (is_et);
    return len;
}

ssize_t HttpConnect::Write() {
    ssize_t len = 0;
    do {
        len = write_buffer.WriteFd(client_sockfd);
    } while (is_et && len > 0);

    assert(len >= 0);
    return len;
}

void HttpConnect::Close() {
    // TODO: Finish it;


}

HttpConnect::HttpConnect() {
    client_sockfd = -1;
    client_sockaddr = {0};
    is_close = true;

}

HttpConnect::HttpConnect(int fd_, const sockaddr_in &addr)
: client_sockfd(fd_), client_sockaddr(addr)
{
    assert(client_sockfd >= 0);

}

HttpConnect::~HttpConnect() {
    Close();
}

//void HttpConnect::Process() {
//    http_request
//
//
//}

