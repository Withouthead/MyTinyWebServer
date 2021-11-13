//
// Created by xiaomaotou31 on 2021/11/12.
//

#ifndef MYTINYSERVER_HTTPRESPONSE_H
#define MYTINYSERVER_HTTPRESPONSE_H

#include "../utils/HttpBuffer.h"
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void MakeResponse(HttpBuffer& buffer);
    void UnmapFile();
    void Init(const std::string& src_dir_, const std::string& path_, bool is_keep_alive_, int code_);

    int getFileLen() const;
private:
    void ErrorContent(HttpBuffer& buffer, const std::string& message) const;
    void AddStateLine(HttpBuffer &buffer);
    void AddHeader(HttpBuffer &buffer);
    void AddContent(HttpBuffer &buffer);
    void ErrorHtml();
    std::string GetFileType();

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int ,std::string> Code_Status;
    static const std::unordered_map<int ,std::string> Code_Path;

    int code;
    bool is_keep_alive;
    std::string path;
    std::string src_dir;
    struct stat mm_file_stat;
    char* mm_file;
public:
    char *getMmFile() const;
};


#endif //MYTINYSERVER_HTTPRESPONSE_H
