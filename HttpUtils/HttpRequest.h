//
// Created by xiaomaotou31 on 2021/11/10.
//

#ifndef MYTINYSERVER_HTTPREQUEST_H
#define MYTINYSERVER_HTTPREQUEST_H

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <mysql/mysql.h>
#include "../utils/HttpBuffer.h"
#include "../utils/ServerLog.h"
#include "../utils/json.hpp"
#include "../utils/MySqlPool.h"
#include "../utils/MySqlPoolRAII.h"


using json = nlohmann::json;
class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH
    };
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    bool Parse(HttpBuffer& buffer);
    void Init();

    bool IsKeepAlive() const;

private:
    HttpBuffer buff;
    PARSE_STATE state;
    std::string http_method;
    std::string http_version;
    std::string http_body;
    std::string http_request_path;

    bool is_login;
public:
    const std::string &getHttpRequestPath() const;

    void setHttpRequestPath(const std::string &httpRequestPath);

private:
    std::unordered_map<std::string, std::string> header_info;
    json post_info;
    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& header);
    void ParseBody(const std::string& body);
    void ParsePath();
    void ParsePost();
    void ParseJson();
    void ParseUrlEncode();
    bool UserAuth(const std::string& name, const std::string& pwd);
    static int ToDec(char c);

    const std::unordered_set<std::string>DEFAULT_HTML
    {
            "/index", "/register", "/login",
            "/welcome", "/video", "/picture",
    };
    const std::unordered_map<std::string, int>DEFAULT_HTML_TAG
            {
                    {"/index", 0}, {"/login", 1}
            };




};


#endif //MYTINYSERVER_HTTPREQUEST_H
