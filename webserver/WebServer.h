//
// Created by xiaomaotou31 on 2021/11/14.
//

#ifndef MYTINYSERVER_WEBSERVER_H
#define MYTINYSERVER_WEBSERVER_H

#include <unordered_map>
#include "../HttpUtils/HttpConnect.h"
#include "../utils/HeapTimer.h"
#include "../utils/ThreadPool.h"
#include "../utils/ServerLog.h"
#include "../utils/MySqlPool.h"
#include "Epoller.h"

class WebServer {
public:
    WebServer(int port_, int trig_mode, int timeout_, bool opt_linger_, int thread_num, const std::string& src_dir_,
    const std::string& sql_username, const std::string& sql_password, const std::string& sql_host, const std::string& database, const int sql_port);
    void StartServer();
    bool InitServerSocket();
    void InitEventMode(int trig_mode);
    static int SetNonBlock(int fd);
    void AcceptClient();
    void ReadFromClient(HttpConnect*);
    void WriteFromClient(HttpConnect*);
    void ExtentTime(HttpConnect* client_connect);
    void ReadTask(HttpConnect*);
    void ProcessTask(HttpConnect *);
    void WriteTask(HttpConnect *);
    void CloseConnect(HttpConnect* client);
    ~WebServer();
private:
    void AddClient(int fd, sockaddr_in& addr);

    std::unordered_map<int, HttpConnect> users;
    HeapTimer timer;
    ThreadPool thread_pool;
    Epoller epoller;

    int port;
    int timeout;
    std::string src_dir;
    int server_fd;
    bool open_linger; // 优雅关闭
    bool is_close;



    uint32_t listen_event;
    uint32_t connect_event;

    static const int MAX_FD = 65536;




};


#endif //MYTINYSERVER_WEBSERVER_H
