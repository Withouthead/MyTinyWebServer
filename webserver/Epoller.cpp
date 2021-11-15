//
// Created by xiaomaotou31 on 2021/11/15.
//

#include "Epoller.h"

Epoller::Epoller(int max_event):events(max_event){
    epoll_fd = epoll_create(max_event);
}

Epoller::~Epoller() {
    close(epoll_fd);
}

bool Epoller::AddFd(int fd, uint32_t events_flag) {
    assert(fd);
    epoll_event event{};
    event.data.fd = fd;
    event.events = events_flag;
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == 0;
}

bool Epoller::ModFd(int fd, uint32_t events_flag) {
    assert(fd);
    epoll_event event{};
    event.data.fd = fd;
    event.events = events_flag;
    return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

bool Epoller::DelFd(int fd) {
    assert(fd);
    return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);

}

int Epoller::Wait(int timeoutMs) {
    return epoll_wait(epoll_fd, &events[0], static_cast<int>(events.size()), timeoutMs);
}

epoll_event Epoller::GetEvent(size_t index) const {
    assert(index < events.size());
    return events[index];
}



