//
// Created by xiaomaotou31 on 2021/11/16.
//

#include "ServerLog.h"

void ServerLog::Init(const std::string& path_, const std::string& suffix_) {
    time_t now_time = time(nullptr);
    system_time = localtime(&now_time);
    char file_name_[1024];
    std::string file_time_prefix = GetFormatTimeFileNamePrefix();
    snprintf(file_name_, 1024, "%s/%s%s", path_.c_str(), file_time_prefix.c_str(), suffix_.c_str());

}

std::string ServerLog::GetFormatTimeFileNamePrefix() {
    char time_name[1024];
    strftime(time_name, sizeof(time_name), "%Y_%m_%d", system_time);
    return std::string(time_name);
}
