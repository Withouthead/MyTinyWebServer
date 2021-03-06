//
// Created by xiaomaotou31 on 2021/11/14.
//

#include "WebServer.h"
int HttpConnect::user_count;
WebServer::WebServer(int port_, int trig_mode, int timeout_, bool opt_linger_, int thread_num, const std::string& src_dir_,
                     const std::string& sql_username, const std::string& sql_password, const std::string& sql_host, const std::string& database, const int sql_port)
                     :port(port_), timeout(timeout_), thread_pool(thread_num), open_linger(opt_linger_){

    ServerLog::Init("./log", ".log");
    ServerLog::LogInfo("Server Init successfully");

    ServerLog::LogInfo("=============INIT WebServer=============");
    ServerLog::LogInfo("Trig Mode: %d", trig_mode);
    ServerLog::LogInfo("Src Dir: %s", src_dir_.c_str());
    ServerLog::LogInfo("Thread Num: %d", thread_num);
    ServerLog::LogInfo("Time Out: %d", timeout_);
    ServerLog::LogInfo("Opt Linger: %s", open_linger ? "Yes" : "No");

    src_dir = src_dir_;
    if(src_dir_.empty())
        src_dir = "./root";

    HttpConnect::user_count = 0;
    HttpConnect::src_dir = src_dir;
    is_close = false;
    InitEventMode(trig_mode);
    is_close = !InitServerSocket();
    MySqlPool::Init(sql_host, sql_port, sql_username, sql_password, database, 20);
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
    if(is_close)
        return false;
    ServerLog::LogInfo("=============INIT ServerSocket===========");
    int ret;
    std::string info = "InitServerSocket: ";
    sockaddr_in addr{};
    if(port > MAX_FD || port < 1024)
        return false;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
    {
        ServerLog::LogError("%s", strerror(errno));
        return false;
    }
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerLog::LogInfo("Server Address: %s", inet_ntoa(addr.sin_addr));
    addr.sin_port = htons(port);
    ServerLog::LogInfo("Server Port %d", port);
    addr.sin_family = AF_INET;

    struct linger opt_linger_{};
    if(open_linger)//????????????
    {
        opt_linger_.l_linger = 1;
        opt_linger_.l_onoff = 1;
    }
    ret = setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &opt_linger_, sizeof(opt_linger_));
    if(ret < 0)
    {
        ServerLog::LogError("%s", strerror(errno));
        return false;
    }
    int opt_val = 1;
    ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt_val, sizeof(opt_val));
    if(ret < 0)
    {
        ServerLog::LogError("%s", strerror(errno));
        return false;
    }

    ret = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0)
    {
        ServerLog::LogError("%s", strerror(errno));
        return false;
    }

    ret = listen(server_fd, 10);
    if(ret < 0)
    {
        ServerLog::LogError("%s", strerror(errno));
        return false;
    }

    ret = epoller.AddFd(server_fd, listen_event | EPOLLIN);
    if(ret < 0)
    {
        ServerLog::LogError("%s", strerror(errno));
        return false;
    }

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
        ServerLog::LogInfo("=============StartServer=============");
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
    ServerLog::LogInfo("Add New Client :%d", fd);
    ServerLog::LogInfo("Address :%s", inet_ntoa(addr.sin_addr));
    users[fd].Init(fd, addr);
    if(timeout > 0)
    {
        timer.AddNde(fd, timeout, [this, capture0 = &users[fd]] { CloseConnect(capture0); });
    }
    epoller.AddFd(fd, EPOLLIN | connect_event);
    SetNonBlock(fd);
}

void WebServer::CloseConnect(HttpConnect *client) {
    if(client->IsClose())
        return;
    assert(client);
    if(client->getClientSockfd() == 27)
    {
        printf("??");
    }
    std::string addr_s(inet_ntoa(client->getClientSockaddr().sin_addr));
    epoller.DelFd(client->getClientSockfd());
    timer.DelNodeByFd(client->getClientSockfd());
    client->Close();
    ServerLog::LogInfo("%d %s Connection closed", client->getClientSockfd(), inet_ntoa(client->getClientSockaddr().sin_addr));
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
    ServerLog::LogInfo("%d %s Send Message", client_connect->getClientSockfd(), inet_ntoa(client_connect->getClientSockaddr().sin_addr));
    ret = client_connect->Read();
    if(ret <= 0)
    {
        CloseConnect(client_connect);
    }
    ProcessTask(client_connect);
}

void WebServer::ProcessTask(HttpConnect * client_connect) {
    assert(client_connect);
    ServerLog::LogInfo("Process %d %s Request", client_connect->getClientSockfd(), inet_ntoa(client_connect->getClientSockaddr().sin_addr));
    if(client_connect->Process())
    {
        epoller.ModFd(client_connect->getClientSockfd(), connect_event | EPOLLOUT);

    }
    else
        epoller.ModFd(client_connect->getClientSockfd(), connect_event | EPOLLIN);
}

void WebServer::WriteTask(HttpConnect * client_connect) {
    assert(client_connect);
    ServerLog::LogInfo("Response to %s", inet_ntoa(client_connect->getClientSockaddr().sin_addr));
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
