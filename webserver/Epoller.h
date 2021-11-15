//
// Created by xiaomaotou31 on 2021/11/15.
//

#ifndef MYTINYSERVER_EPOLLER_H
#define MYTINYSERVER_EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <vector>

class Epoller {
public:
    explicit Epoller(int max_event=1024);
    ~Epoller();
    bool AddFd(int fd, uint32_t events_flag);
    bool ModFd(int fd, uint32_t events_flag);
    bool DelFd(int fd);
    int Wait(int timeoutMs = -1);
    epoll_event GetEvent(size_t index) const;

private:
    int epoll_fd;
    std::vector<struct epoll_event> events;
};


#endif //MYTINYSERVER_EPOLLER_H
