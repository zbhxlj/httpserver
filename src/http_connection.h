#pragma once

#include "noncopyable.h"
#include "socket.h"
#include <memory>
#include <string>
namespace webserver {

class EventLoop;
class Channel;
class HttpHandler;
/* Abstract of a http connection.
   One HttpHandler hold a HttpConnection.
   In charge of connection's open、read、write、close， and corresponding state.
 */
class HttpConnection : public Noncopyable {

  public:
    using socket_ptr = std::shared_ptr<TcpSocket>;
    using channel_ptr = std::shared_ptr<Channel>;
    enum ConnState { Connected, Handle, Error, DisConnecting, DisConnected };

    HttpConnection(EventLoop *loop, socket_ptr conn_fd);
    ~HttpConnection();

    void handle_read();
    void handle_write();
    void handle_close();
    void send(const void *data, int length);
    void send(const std::string &data);
    channel_ptr &get_channel() { return m_channel; }

    void set_default_cb();
    std::string get_recv_buffer() {
        std::string buf;
        buf.swap(m_in_buffer);
        return buf;
    }
    void set_handler(const std::shared_ptr<HttpHandler> &handler) {
        m_handler = handler;
    }

    ConnState get_state() const { return m_state; }
    void set_state(ConnState state) { m_state = state; }
    void shut_down(int how);

  private:
    EventLoop *m_loop;
    socket_ptr m_conn_fd;
    channel_ptr m_channel;
    std::string m_in_buffer;
    std::string m_out_buffer;
    std::weak_ptr<HttpHandler> m_handler;
    ConnState m_state;
};

} // namespace webserver