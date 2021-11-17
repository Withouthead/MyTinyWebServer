//
// Created by xiaomaotou31 on 2021/11/16.
//

#include "ServerLog.h"
#define LOG_MAX 65536
void ServerLog::Init(const std::string& path_, const std::string& suffix_)
{
    ServerLog *p = GetInstance();
    p->file_name_suffix = suffix_;
    p->file_path = path_;
    if(!std::filesystem::exists(p->file_path))
    {
        std::filesystem::create_directories(p->file_path);
    }
    time_t now_time = time(nullptr);
    p->system_time = localtime(&now_time);

    if(p->write_thread == nullptr)
    {
        std::unique_ptr<std::thread> temp_pointer(new std::thread(WriteThreadFun));
        p->write_thread = std::move(temp_pointer);
    }
    p->file_name = p->GetFileNameBaseSystime();
    p->file_path = p->file_path / std::filesystem::path(p->file_name);
    p->log_file = fopen(p->file_path.c_str(), "w");
    assert(p->log_file);

}

std::string ServerLog::GetFormatTimeFileNamePrefix() const{
    char time_name[1024];
    strftime(time_name, sizeof(time_name), "%Y_%m_%d", system_time);
    return std::string(time_name);
}

void ServerLog::Write(int level, char *format, va_list msg_va_list) {

    std::unique_lock<std::mutex> lock(log_mutex);
    std::string log_prefix = GetLevel(level);
    char msg[LOG_MAX];
    memset(msg, 0, sizeof(msg));
    vsnprintf(msg, LOG_MAX, format, msg_va_list);
    time_t now_time = time(nullptr);
    struct tm* now_time_tm = localtime(&now_time);

    if(now_time_tm->tm_mday != system_time->tm_mday)
    {
        std::unique_lock<std::mutex> file_lock(file_mutex);
        *system_time = *now_time_tm;
        Flush();
        fclose(log_file);
        file_name = GetFileNameBaseSystime();
        file_path.replace_filename(file_name);
        log_file = fopen(file_path.c_str(), "w");
    }

    std::string msg_to_queue;
    msg_to_queue = GetFormatTimeLinePrefix() + " " + log_prefix + "    :" + msg + "\n";
    bool is_success = log_queue.Push(msg_to_queue);


}

std::string ServerLog::GetLevel(int level) const {
    return LOG_LEVEL[level];
}

std::string ServerLog::GetFileNameBaseSystime() const {
    char file_name_[LOG_MAX];
    std::string file_time_prefix = GetFormatTimeFileNamePrefix();
    snprintf(file_name_, 1024, "%s%s", file_time_prefix.c_str(), file_name_suffix.c_str());
    return std::string(file_name_);
}

std::string ServerLog::GetFormatTimeLinePrefix() const {
    char time_string [128];
    time_t now_time = time(nullptr);
    struct tm* now_tm = localtime(&now_time);
    strftime(time_string ,sizeof(time_string ),"%Y-%m-%e %H:%M:%S", now_tm);
    return std::string(time_string);
}

void ServerLog::WriteAsynchronous() {
    std::string msg_to_write;
    while(log_queue.Pop(msg_to_write))
    {
        std::unique_lock<std::mutex> lock(file_mutex);
        fputs(msg_to_write.c_str(), log_file);
        fflush(log_file);
    }

}
ServerLog *ServerLog::GetInstance() {
    static ServerLog p;
    return &p;
}

void ServerLog::WriteThreadFun() {
    ServerLog::GetInstance()->WriteAsynchronous();
}

ServerLog::ServerLog() {
    write_thread = nullptr;
    log_file = nullptr;
    system_time = nullptr;
}

ServerLog::~ServerLog() {
    if(write_thread && write_thread->joinable())
    {
        while(!log_queue.empty())
        {
            log_queue.flush();
        }
        log_queue.Close();
        write_thread->join();
    }
    if(log_file)
    {
        std::unique_lock<std::mutex> lock(log_mutex);
        Flush();
        fclose(log_file);
    }
}

void ServerLog::Flush() {
    assert(log_file);
    log_queue.flush();
    fflush(log_file);

}

void ServerLog::LogInfo(std::string format, ...) {

    ServerLog* p = GetInstance();
    va_list msg_va_list;
    char* c_format = format.data();
    va_start(msg_va_list, format);
    p->Write(0, c_format, msg_va_list);
    va_end(msg_va_list);
}

void ServerLog::LogDeBug(char *format, ...) {
    ServerLog* p = GetInstance();
    va_list msg_va_list;
    va_start(msg_va_list, format);
    p->Write(1, format, msg_va_list);
    va_end(msg_va_list);
}

void ServerLog::LogWarning(char *format, ...) {
    ServerLog* p = GetInstance();
    va_list msg_va_list;
    va_start(msg_va_list, format);
    p->Write(2, format, msg_va_list);
    va_end(msg_va_list);
}

void ServerLog::LogError(char *format, ...) {
    ServerLog* p = GetInstance();
    va_list msg_va_list;
    va_start(msg_va_list, format);
    p->Write(3, format, msg_va_list);
    va_end(msg_va_list);
}
