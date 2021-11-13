//
// Created by xiaomaotou31 on 2021/11/12.
//

#include "HttpResponse.h"
const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
        {".html", "text/html"},
        {".xml", "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt", "text/plain"},
        {".rtf", "application/rtf"},
        {".pdf", "application/pdf"},
        {".word", "application/nsword"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".au", "audio/basic"},
        {".mpeg", "video/mpeg"},
        {".mpg", "video/mpeg"},
        {".avi", "video/x-msvideo"},
        {".gz", "application/x-gzip"},
        {".tar", "application/x-tar"},
        {".css", "text/css"},
        {".js", "text/javascript"}
};

const std::unordered_map<int ,std::string> HttpResponse::Code_Status = {
        {200, "OK"},
        {400, "Bad Request"},
        {403, "Forbidden"},
        {404, "Not Found"}
};

const std::unordered_map<int, std::string> HttpResponse::Code_Path = {
        {400, "/400.html"},
        {403, "/403.html"},
        {404, "/404.html"}
};

void HttpResponse::AddStateLine(HttpBuffer &buffer) {
    std::string status;
    if(Code_Status.count(code))
        status = Code_Status.find(code)->second;
    else
    {
        code = 400;
        status = Code_Status.find(400)->second;
    }
    buffer.Append("HTTP/1.1 " + std::to_string(code) + " " + status + "\r\b");

}

void HttpResponse::AddHeader(HttpBuffer &buffer) {
    buffer.Append("Connection: ");//是否为持续连接
    if(is_keep_alive)
    {
        buffer.Append("keep-alive\r\n");
        buffer.Append("keep-alive: max=6, timeout=120\r\n");

    }
    else
    {
        buffer.Append("close\r\n");
    }
    buffer.Append("Content-type: " + GetFileType() + "\r\n");
}

std::string HttpResponse::GetFileType() {
    std::string::size_type index = path.find_last_of('.');
    if(index == std::string::npos)
        return "text/plain";
    std::string suffix = path.substr(index);
    if(SUFFIX_TYPE.count(suffix))
        return SUFFIX_TYPE.find(suffix)->second;
    return "text/plain";
}

void HttpResponse::AddContent(HttpBuffer &buffer) {
    int src_fd = open((src_dir + path).data(), O_RDONLY);
    if (src_fd < 0) {
        ErrorContent(buffer, "File Not Found!");
        return;
    }
    int *mm_point = (int *) mmap(nullptr, mm_file_stat.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    mm_file = (char *) mm_point;
    close(src_fd);
    buffer.Append("Content-length: " + std::to_string(mm_file_stat.st_size));
}

void HttpResponse::UnmapFile() {
    if(mm_file)
    {
        munmap(mm_file, mm_file_stat.st_size);
        mm_file = nullptr;
    }
}

void HttpResponse::ErrorContent(HttpBuffer &buffer, const std::string& message) {
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(Code_Status.count(code))
        status = Code_Status.find(code)->second;
    else
        status = "Bad Request";
    body += std::to_string(code) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>MyTinyWebServer</em></body></html>";

    buffer.Append("Content-length :" + std::to_string(body.size()) + "\r\n\r\n");
    buffer.Append(body);
}
