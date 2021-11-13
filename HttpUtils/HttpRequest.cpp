//
// Created by xiaomaotou31 on 2021/11/10.
//

#include "HttpRequest.h"


bool HttpRequest::ParseRequestLine(const std::string& line) {
    if(state != PARSE_STATE::REQUEST_LINE)
        return false;
    std::regex request_line_regex("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_match;
    if(std::regex_match(line, sub_match, request_line_regex))
    {
        http_method = sub_match[1];
        http_version = sub_match[2];
        http_request_path = sub_match[3];
        state = HEADER;
        return true;
    }
    return false;
}

void HttpRequest::ParseHeader(const std::string& header) {
    std::regex request_header_regex("^([^:]) ?(.*)$");
    std::smatch sub_match;
    if(std::regex_match(header, sub_match, request_header_regex))
    {
        header_info[sub_match[1]] = sub_match[2];
    }
    else
        state = BODY;
}

void HttpRequest::ParseBody(const std::string& body) {
    http_body = body;
    state = FINISH;
}

bool HttpRequest::Parse(HttpBuffer &buffer) {
    const std::string CRLF = "\r\n";
    while (state != FINISH && buffer.UsableSize()) {
        size_t crlf_index = buffer.SearchSubString(CRLF);
        std::string line(buffer.GetStringFromReadBuffer(crlf_index + CRLF.size() - 1));
        line = line.substr(0, line.size() - CRLF.size());
        switch (state) {
            case REQUEST_LINE:
                if (!ParseRequestLine(line))
                    return false;
                ParsePath();
                break;
            case HEADER:
                ParseHeader(line);
                if (buffer.UsableSize() <= 0)
                    state = FINISH;
                break;
            case BODY:
                ParseBody(line);
                break;
            default:
                break;
        }
    }

    return true;
}

void HttpRequest::ParsePath() {
    if(http_request_path == "/")
        http_request_path = "/index.html";

    else
    {
        auto ite = DEFAULT_HTML.find(http_request_path);
        if(ite != DEFAULT_HTML.end())
            http_request_path += ".html";
        else
            http_request_path = "/404.html";
    }

}

bool HttpRequest::IsKeepAlive() const {
    if (header_info.find("Connection") != header_info.end())
    {
        return header_info.find("Connection")->second == "keep-alive" && http_version == "1.1";
    }
    return false;
}

void HttpRequest::Init() {
    buff.ClearAllBuffer();
    http_method.clear();
    http_version.clear();
    http_request_path.clear();
    state = REQUEST_LINE;
    header_info.clear();

}

const std::string &HttpRequest::getHttpRequestPath() const {
    return http_request_path;
}

void HttpRequest::setHttpRequestPath(const std::string &httpRequestPath) {
    http_request_path = httpRequestPath;
}
