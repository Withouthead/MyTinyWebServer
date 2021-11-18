//
// Created by xiaomaotou31 on 2021/11/18.
//

#include "MySqlPool.h"

MySqlPool *MySqlPool::GetInstance() {
    static MySqlPool p;
    return &p;
}

void MySqlPool::Init(const std::string& host, int port, const std::string& user, const std::string pwd,
                     const std::string& database, int connect_size) {
    assert(connect_size);
    ServerLog::LogDeBug("=============INIT MySqlPool===============");
    for(int i = 0; i < connect_size; i++)
    {
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if(!sql)
        {
            ServerLog::LogError("Mysql Init Error");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host.c_str(), user.c_str(), pwd.c_str(), database.c_str(), port, nullptr, 0);
        if(!sql)
        {
            ServerLog::LogError("Mysql Init Error");
            assert(sql);
        }
        connection_queue.push(sql);
    }
    max_connect_size = connect_size;
}

MYSQL *MySqlPool::GetConnection() {
    std::unique_lock<std::mutex> lock(pool_mutex);
    if(connection_queue.empty())
    {
        ServerLog::LogWarning("Sql Pool is empty!");
        return nullptr;
    }
    cv_consumer.wait(lock, [this]{return !connection_queue.empty();});
    MYSQL *sql = connection_queue.front();
    connection_queue.pop();
    cv_producer.notify_one();
    return sql;
}

void MySqlPool::FreeConnection(MYSQL *sql) {
    std::unique_lock<std::mutex> lock(pool_mutex);
    cv_producer.wait(lock, [this]{return max_connect_size > connection_queue.size();});
    connection_queue.push(sql);
    cv_consumer.notify_one();
}

void MySqlPool::ClosePool() {
    std::unique_lock<std::mutex> lock(pool_mutex);
    while(!connection_queue.empty())
    {
        auto item = connection_queue.front();
        connection_queue.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int MySqlPool::GetFreeConnectionNum(){
    std::unique_lock<std::mutex> lock(pool_mutex);
    return max_connect_size - connection_queue.size();
}

MySqlPool::~MySqlPool() {
    ClosePool();
}


