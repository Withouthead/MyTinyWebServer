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

void HttpResponse::ErrorContent(HttpBuffer &buffer, const std::string& message) const {
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

void HttpResponse::Init(const std::string& src_dir_, const std::string& path_, bool is_keep_alive_, int code_) {
    assert(!src_dir_.empty());
    src_dir = src_dir_;
    assert(!path_.empty());
    path = path_;
    is_keep_alive = is_keep_alive_;
    code = code_;
}

void HttpResponse::MakeResponse(HttpBuffer &buffer) {
    if(stat((src_dir + path).data(), &mm_file_stat) < 0 || S_ISDIR(mm_file_stat.st_mode))
    {
        code = 404;
    }
    else if(!(mm_file_stat.st_mode & S_IROTH))
    {
        code = 403;
    }
    else if(code == -1)
        code == 200;
    ErrorHtml();
    AddStateLine(buffer);
    AddHeader(buffer);
    AddContent(buffer);

}

void HttpResponse::ErrorHtml() {
    if(Code_Path.count(code))
    {
        path = Code_Path.find(code)->second;
        stat((src_dir + path).data(), &mm_file_stat);
    }
}

char *HttpResponse::getMmFile() const {
    return mm_file;
}

int HttpResponse::getFileLen() const {
    return mm_file_stat.st_size;
}

HttpResponse::~HttpResponse() {
    UnmapFile();
}

HttpResponse::HttpResponse() {
    code = -1;
    path = src_dir = "";
    is_keep_alive = false;
    mm_file = nullptr;
    mm_file_stat = {0};

}
