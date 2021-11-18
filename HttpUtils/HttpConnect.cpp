//
// Created by xiaomaotou31 on 2021/11/10.
//

#include "HttpConnect.h"

#include <utility>

bool HttpConnect::is_et;
std::string HttpConnect::src_dir;
ssize_t HttpConnect::Read() {
    ssize_t len = 0;
    do {
        int temp_len = read_buffer.ReadFd(client_sockfd);
        if (temp_len <= 0)
            break;
        len += temp_len;
    } while (HttpConnect::is_et);
    return len;
}

ssize_t HttpConnect::Write(int& write_error) {
    ssize_t len = 0;
    do {
        len = writev(client_sockfd, iov, iov_cnt);
        if(len <= 0) {
            write_error = errno;
            break;
        }
        if(iov[0].iov_len + iov[1].iov_len == 0)
            break;
        else if(static_cast<size_t>(len) > iov[0].iov_len)//
        {
            iov[1].iov_base = (uint8_t*) iov[1].iov_base + (len - iov[0].iov_len);
            iov[1].iov_len -= (len - iov[0].iov_len);
            if(iov[0].iov_len)
            {
                write_buffer.ClearAllBuffer();
                iov[0].iov_len = 0;
            }

        }
        else
        {
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len;
            iov[0].iov_len -= len;
            write_buffer.ClearBuffByLen(len);
        }
    } while (is_et && len > 0 || write_buffer.UsableSize() > 10240);

    return len;
}

void HttpConnect::Close() {
    if(is_close)
        return;
    is_close = true;

    http_response.UnmapFile();
    close(client_sockfd);


}

HttpConnect::HttpConnect() {
    client_sockfd = -1;
    client_sockaddr = {0};
    is_close = true;


}


HttpConnect::~HttpConnect() {
    http_response.UnmapFile();
}

bool HttpConnect::Process() {
    http_request.Init();
    if (read_buffer.UsableSize() <= 0)
        return false;
    if (http_request.Parse(read_buffer)) {
        http_response.Init(src_dir, http_request.getHttpRequestPath(), http_request.IsKeepAlive(), 200);
    } else
        http_response.Init(src_dir, http_request.getHttpRequestPath(), false, 400);
    http_response.MakeResponse(write_buffer);
    iov[0].iov_base = const_cast<char *>(write_buffer.data());
    iov[0].iov_len = write_buffer.UsableSize();
    iov_cnt = 1;
    if(http_response.getFileLen() > 0 && http_response.getMmFile())
    {
        iov[1].iov_base = http_response.getMmFile();
        iov[1].iov_len = http_response.getFileLen();
        iov_cnt = 2;
    }

    return true;
}

void HttpConnect::Init(int client_fd, sockaddr_in client_addr) {
    client_sockfd = client_fd;
    client_sockaddr = client_addr;
    read_buffer.ClearAllBuffer();
    write_buffer.ClearAllBuffer();
    memset(iov, 0, sizeof(iov));
    is_close = false;
}

HttpConnect::HttpConnect(int client_fd, sockaddr_in client_addr) {
    Init(client_fd, client_addr);
}

int HttpConnect::getClientSockfd() const {
    return client_sockfd;
}

int HttpConnect::ToWriteSize() const {
    return write_buffer.UsableSize();
}

bool HttpConnect::IsKeepAlive() const {
    return http_request.IsKeepAlive();
}

const sockaddr_in &HttpConnect::getClientSockaddr() const {
    return client_sockaddr;
}

bool HttpConnect::IsClose() const {
    return is_close;
}

