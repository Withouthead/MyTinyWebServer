//
// Created by xiaomaotou31 on 2021/11/18.
//

#ifndef MYTINYSERVER_MYSQLPOOL_H
#define MYTINYSERVER_MYSQLPOOL_H

#include <mysql/mysql.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include "ServerLog.h"

class MySqlPool {
public:
    static MySqlPool* GetInstance();
    static void Init(const std::string& host, int port, const std::string& user, std::string pwd, const std::string& database, int connect_size);
    MYSQL* GetConnection();
    void FreeConnection(MYSQL* sql);

    int GetFreeConnectionNum();
    ~MySqlPool();

private:
    void ClosePool();
    size_t max_connect_size;
    std::queue<MYSQL*> connection_queue;
    std::condition_variable cv_producer;
    std::condition_variable cv_consumer;
    std::mutex pool_mutex;
};


#endif //MYTINYSERVER_MYSQLPOOL_H
