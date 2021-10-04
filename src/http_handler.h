#pragma once

#include "http_manager.h"
#include "noncopyable.h"
#include "socket.h"
#include <map>
#include <memory>
#include <string>

namespace webserver {

class HttpConnection;
class EventLoop;
class HttpManager;
/* Process Http requests and send response.
   Per HttpHandler owns a HttpConnection.
 */
class HttpHandler : public std::enable_shared_from_this<HttpHandler>,
                    public Noncopyable {

  public:
    using socket_ptr = std::shared_ptr<TcpSocket>;
    friend class HttpManager;
    enum HttpVersion { HttpV10, HttpV11 };
    enum HttpMethod { GET };
    enum HttpState {
        Start,
        ParseRequestLine,
        ParseHeader,
        ParseDone,
        Response
    };
    /* Convert string into enum vaues.
     */
    static const char *Method[];
    static const char *Version[];

    HttpHandler(EventLoop *loop, socket_ptr conn_fd);
    ~HttpHandler();

    /* Add new http connection.
     */
    void new_connection();
    /* Parse http request.
     */
    void handle_http_request();

  private:
    int parse_request_line(std::string &buf, int bpos);
    int parse_header(std::string &buf, int bpos);
    /* Send http response.
     */
    void respond_request();
    /* Clean up after parsing a http request.
       If it is not keep-alive, close it.
       Else reset state, ready for next http request.
     */
    void handle_keep_alive();
    /* Error occured.
       Send bad response.
     */
    void bad_request(int num, const std::string &note);
    /* EveryThing fine.
     */
    void on_request(const std::string &body);

    void set_method(const std::string &method);
    void set_url(const std::string &url);
    void set_version(const std::string &version);
    void set_header(const std::string &key, const std::string &value);
    void reset();
    EventLoop *m_loop;
    socket_ptr m_conn_fd;
    std::unique_ptr<HttpConnection> m_connection;
    HttpMethod m_method;
    HttpVersion m_version;
    HttpState m_state;
    std::map<std::string, std::string> m_headers;
    std::string m_url;
    bool m_is_keep_alive;
    /* Node in keep-alive list.
     */
    HttpManager::timer_node m_timer_node;
};

} // namespace webserver