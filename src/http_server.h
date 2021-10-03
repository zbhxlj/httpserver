#pragma once

#include <memory>
#include "noncopyable.h"
#include "event_loop_thread_pool.h"
#include "inet_addr.h"
#include "socket.h"
namespace webserver{ 

class Channel;
/*  Wrapper of a http server.
 */
class HttpServer : public Noncopyable{

public:
    /* @param listen ip and port
     */
    HttpServer(EventLoop *loop, std::string ip, short port, int thread_nums);
    ~HttpServer();

    void start();
    void acceptor();

private:
    EventLoop* m_base_loop;
    int m_thread_nums;
    std::unique_ptr<EventLoopThreadPool> m_thread_pool;
    TcpSocket m_listen_fd;
    std::shared_ptr<Channel> m_accept_channel;
    bool m_is_started;
    int m_idle_fd;
};

}