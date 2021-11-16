//
// Created by xiaomaotou31 on 2021/11/16.
//

#ifndef MYTINYSERVER_SERVERLOG_H
#define MYTINYSERVER_SERVERLOG_H

#include <chrono>
#include <queue>
#include <thread>
#include <cstdarg>
#include <ctime>
#include "BlockQueue.h"

class ServerLog {
public:
    static ServerLog* GetInstance();

    void LogInfo();
    void LogBug();
    void LogError();
private:
    void WriteAsynchronous();
    std::string GetLevel(int level) const;
    void Write(int level, char* format, ...);
    void Init();
    ServerLog();
    std::string GetFormatTimeLinePrefix();
    std::string GetFormatTimeFileNamePrefix();
    std::thread write_thread;
    BlockQueue<std::string> log_queue;
    std::mutex log_mutex;
    struct tm* today_date;
    struct tm* system_time;
    FILE *log_file;
    std::string file_name;


};


#endif //MYTINYSERVER_SERVERLOG_H
