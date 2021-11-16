//
// Created by xiaomaotou31 on 2021/11/14.
//

#include "WebServer.h"
int HttpConnect::user_count;
WebServer::WebServer(int port_, int trig_mode, int timeout_, bool opt_linger_, int thread_num, const std::string& src_dir_)
:port(port_), timeout(timeout_), thread_pool(thread_num), open_linger(opt_linger_){
    if(src_dir_.empty())
        src_dir = "./root";
    HttpConnect::user_count = 0;
    HttpConnect::src_dir = src_dir;
    InitEventMode(trig_mode);
    InitServerSocket();
    is_close = false;


}
WebServer::~WebServer() {
    close(server_fd);
}
void WebServer::InitEventMode(int trig_mode) {
    listen_event = EPOLLRDHUP;
    connect_event = EPOLLONESHOT | EPOLLRDHUP;
    switch (trig_mode) {
        case 0:
            break;
        case 1:
            connect_event |= EPOLLET;
            break;
        case 2:
            listen_event |= EPOLLET;
            break;
        case 3:
            listen_event |= EPOLLET;
            connect_event |= EPOLLET;
            break;
        default:
            listen_event |= EPOLLET;
            connect_event |= EPOLLET;
            break;
    }
    HttpConnect::is_et = connect_event & EPOLLET;
}

bool WebServer::InitServerSocket() {
    int ret;
    sockaddr_in addr{};
    if(port > MAX_FD || port < 1024)
        return false;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    struct linger opt_linger_{};
    if(open_linger)//优雅关闭
    {
        opt_linger_.l_linger = 1;
        opt_linger_.l_onoff = 1;
    }
    ret = setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &opt_linger_, sizeof(opt_linger_));
    assert(ret >= 0);
    int opt_val = 1;
    ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt_val, sizeof(opt_val));
    assert(ret >= 0);

    ret = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    assert(ret >=0);

    ret = listen(server_fd, 10);
    assert(ret >= 0);

    ret = epoller.AddFd(server_fd, listen_event | EPOLLIN);
    assert(ret >= 0);

    SetNonBlock(server_fd);
    return true;

}

int WebServer::SetNonBlock(int fd) {
    assert(fd >= 0);
    int old_opt = fcntl(fd, F_GETFL, 0);
    int new_opt = old_opt | O_NONBLOCK;
    return fcntl(fd, F_SETFL, new_opt);
}

void WebServer::StartServer() {
    int timeMs = -1;
    if(!is_close)
    {
        while(!is_close)
        {
            if(timeout > 0)
            {
                timeMs = timer.GetTopTick();
            }
            int event_cnt = epoller.Wait(timeMs);
            for(int i = 0; i < event_cnt; i++)
            {
                epoll_event event = epoller.GetEvent(i);
                int fd = event.data.fd;
                int event_flag = event.events;
                if(fd == server_fd)
                {
                    std::cout << "new connect come in " << std::endl;
                    AcceptClient();
                }
                else if(event_flag & EPOLLIN)
                {
                    assert(users.count(fd));
                    ReadFromClient(&users[fd]);
                }
                else if(event_flag & EPOLLOUT)
                {
                    assert(users.count(fd) > 0);
                    WriteFromClient(&users[fd]);
                }

            }
        }
    }
}

void WebServer::AcceptClient() {
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    do {
        int client_fd = accept(server_fd, (struct sockaddr *)&addr, &len);
        if(client_fd < 0)
            return;
        if(HttpConnect::user_count >= MAX_FD)
            return;
        AddClient(client_fd, addr);
    } while (listen_event & EPOLLET);
}

void WebServer::AddClient(int fd, sockaddr_in &addr) {
    assert(fd >= 0);
    users[fd].Init(fd, addr);
    if(timeout > 0)
    {
        timer.AddNde(fd, timeout, [this, capture0 = &users[fd]] { CloseConnect(capture0); });
    }
    epoller.AddFd(fd, EPOLLIN | connect_event);
    SetNonBlock(fd);
}

void WebServer::CloseConnect(HttpConnect *client) {
    assert(client);
    epoller.DelFd(client->getClientSockfd());
    client->Close();
}

void WebServer::ReadFromClient(HttpConnect * client_connect) {
    assert(client_connect);
    ExtentTime(client_connect);
    thread_pool.enqueue([this, client_connect] { ReadTask(client_connect);});
}



void WebServer::ExtentTime(HttpConnect *client_connect) {
    assert(client_connect);
    if(timeout > 0)
    {
        timer.Update(client_connect->getClientSockfd(), timeout);
    }
}

void WebServer::ReadTask(HttpConnect * client_connect) {
    int ret = -1;
    ret = client_connect->Read();
    if(ret <= 0)
    {
        CloseConnect(client_connect);
    }
    ProcessTask(client_connect);
}

void WebServer::ProcessTask(HttpConnect * client_connect) {
    assert(client_connect);
    if(client_connect->Process())
    {
        epoller.ModFd(client_connect->getClientSockfd(), connect_event | EPOLLOUT);

    }
    else
        epoller.ModFd(client_connect->getClientSockfd(), connect_event | EPOLLIN);
}

void WebServer::WriteTask(HttpConnect * client_connect) {
    assert(client_connect);
    int ret = -1;
    int write_error = 0;
    ret = client_connect->Write(write_error);
    if(client_connect->ToWriteSize() == 0)
    {
        if(client_connect->IsKeepAlive())
        {
            ProcessTask(client_connect);
            return;
        }
    }
    else if(ret < 0)
    {
        if(write_error == EAGAIN)
        {
            epoller.ModFd(client_connect->getClientSockfd(), connect_event | EPOLLOUT);
            return;
        }
    }
    CloseConnect(client_connect);
}

void WebServer::WriteFromClient(HttpConnect * client_connect) {
    assert(client_connect);
    ExtentTime(client_connect);
    thread_pool.enqueue([this, client_connect]{WriteTask(client_connect);});

}
