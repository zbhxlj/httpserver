#pragma once

#include <string>
#include <map>
#include "http_manager.h"
#include "socket.h"
#include <memory>

namespace webserver{

class HttpConnection;
class EventLoop;
class HttpManager;
class HttpHandler : public std::enable_shared_from_this<HttpHandler>{

public:
    friend class HttpManager;
    enum HttpVersion {HttpV10, HttpV11};
    enum HttpMethod {GET};
    enum HttpState {Start, ParseRequestLine, ParseHeader, ParseBody, ParseDone, Response};

    static const char* Method[];
    static const char* Version[];

    HttpHandler(EventLoop* loop, TcpSocket conn_fd);
    ~HttpHandler();

    void new_connection();
    void handle_http_request();
private:
    int parse_request_line(std::string& buf, int bpos);
    int parse_header(std::string& buf, int bpos);
    void respond_request();
    void handle_keep_alive();
    void bad_request(int num, const std::string& note);
    void on_request(const std::string& body);

    void set_method(const std::string& method);
    void set_url(const std::string& url);
    void set_version(const std::string& version);
    void set_header(const std::string& key, const std::string& value);
    void reset();
    EventLoop* m_loop;
    TcpSocket m_conn_fd;
    std::unique_ptr<HttpConnection> m_connection;
    HttpMethod m_method;
    HttpVersion m_version;
    HttpState m_state;
    std::map<std::string, std::string> m_headers;
    std::string m_url;
    bool m_is_keep_alive;
    HttpManager::timer_node m_timer_node;
};

}