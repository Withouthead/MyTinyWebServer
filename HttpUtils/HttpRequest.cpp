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
        http_version = sub_match[3];
        http_request_path = sub_match[2];
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
    is_login = false;
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

void HttpRequest::ParsePost() {
    if(http_method != "POST")
        return;
    if(header_info["Content-Type"] == "application/x-www-form-urlencoded")
        ParseUrlEncode();
    if(header_info["Content-Type"] == "application/json")
        ParseUrlEncode();
    else
    {
        ServerLog::LogError("Only support json and urlencoded!");
        return;
    }
    if(DEFAULT_HTML_TAG.count(http_request_path))
    {
        int tag = DEFAULT_HTML_TAG.find(http_request_path)->second;
        if(tag == 0 || tag == 1)
        {
            is_login = (tag == 1);
            if(UserAuth(post_info["username"], post_info["password"]))
            {
                http_request_path = "/welcome.html";
            } else
                http_request_path = "/error.html";
        }

    }


}

void HttpRequest::ParseUrlEncode() {

    if(http_body.empty())
        return;
    ServerLog::LogDeBug("ParseUrlEncode Start");
    int size = http_body.size();
    int i = 0;
    std::vector<std::string> key_value(2);
    int key_value_index = 0;
    while(i < size)
    {
        switch (http_body[i]) {
            case '=':
                post_info[key_value[0]] = "";
                key_value_index = (key_value_index + 1) % 2;
                i ++;
                break;
            case '%':
                key_value[key_value_index] += char(ToDec(http_body[i + 1]) * 16 + ToDec(http_body[i + 2]));
                i += 2;
                break;
            case '&':
                post_info[key_value[0]] = key_value[1];
                key_value_index = key_value_index = (key_value_index + 1) % 2;
                i ++;
                break;
            default:
                key_value[key_value_index] += http_body[i];

        }
    }
    ServerLog::LogDeBug("ParseUrlEncode End");
}

int HttpRequest::ToDec(char c) {

    if(c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    ServerLog::LogWarning("This character %c is not hexadecimal", c);
    return 0;
}

void HttpRequest::ParseJson() {
    post_info = http_body;
    ServerLog::LogDeBug("ParseJson successfully!");
}

bool HttpRequest::UserAuth(const std::string &name, const std::string &pwd) {
    if(name.empty() || pwd.empty())
        return false;

    bool is_pwd_correct = false;
    bool username_has_been_used = false;
    ServerLog::LogInfo("Verify User: %s", name.c_str());
    auto sql_raii = MySqlPoolRaII();
    MYSQL_RES *res = nullptr;
    assert(sql_raii.getSqlConnection());
    char sql_order[256];
    snprintf(sql_order, 256, "select username, password from user where username = '%s' limit 1", name.c_str());
    ServerLog::LogDeBug("%s", sql_order);
    if(mysql_query(sql_raii.getSqlConnection(), sql_order))
    {
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql_raii.getSqlConnection());
    while(MYSQL_ROW row = mysql_fetch_row(res))
    {
        ServerLog::LogDeBug("Mysql Row: %s %s", row[0], row[1]);
        std::string  password(row[1]);
        if(pwd == password)
        {
            is_pwd_correct = true;
            ServerLog::LogInfo("Password right!");
        }
        else
        {
            is_pwd_correct = false;
            ServerLog::LogInfo("Password wrong!");
        }
        username_has_been_used = true;
    }
    mysql_free_result(res);
    if(!is_login && !username_has_been_used)
    {
        ServerLog::LogDeBug("User Register!");
        memset(sql_order, 0, sizeof(sql_order));
        snprintf(sql_order, 256, "insert into user(username, password) values('%s', '%s')", name.c_str(), pwd.c_str());
        is_pwd_correct = true;
    }
    ServerLog::LogInfo("User authentication succeeded");
    return is_pwd_correct;

}
