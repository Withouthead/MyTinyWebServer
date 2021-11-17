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
#include <sys/stat.h>
#include <fcntl.h>
#include <filesystem>
#include <cstring>
#include "BlockQueue.h"

class ServerLog {
public:
    static ServerLog* GetInstance();
    ~ServerLog();
    static void LogInfo(std::string format, ...);
    static void LogDeBug(char* format, ...);
    static void LogWarning(char* format, ...);
    static void LogError(char* format, ...);
    static void WriteThreadFun();
    static void Init(const std::string& path_, const std::string& suffix_);
private:
    void WriteAsynchronous();
    std::string GetLevel(int level) const;
    void Write(int level, char* format, va_list msg_va_list);

    ServerLog();
    std::string GetFormatTimeLinePrefix() const;
    std::string GetFileNameBaseSystime() const;
    std::string GetFormatTimeFileNamePrefix() const;
    std::unique_ptr<std::thread>  write_thread;
    BlockQueue<std::string> log_queue;
    std::mutex log_mutex;
    std::mutex file_mutex;

    struct tm* system_time;
    FILE *log_file;
    std::string file_name;
    std::string file_name_suffix;
    std::filesystem::path file_path;

    void Flush();
    const std::string  LOG_LEVEL[5] = {
            "[INFO]",
            "[DEBUG]",
            "[WARNING]",
            "[ERROR]"
    };


};


#endif //MYTINYSERVER_SERVERLOG_H
