#include "http_server.h"
#include "channel.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "http_handler.h"
#include "socket.h"
#include <cassert>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

namespace webserver {
HttpServer::HttpServer(EventLoop *loop, std::string ip, short port,
                       int thread_nums)
    : m_base_loop(loop), m_thread_nums(thread_nums),
      m_thread_pool(
          std::make_unique<EventLoopThreadPool>(m_base_loop, m_thread_nums)),
      m_listen_fd(std::make_shared<TcpSocket>(ip, port)),
      m_accept_channel(std::make_shared<Channel>(m_listen_fd, m_base_loop)),
      m_is_started(false),
      m_idle_fd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(m_listen_fd->get_fd() > 0);
    assert(m_idle_fd > 0);

    m_listen_fd->ignore_sigpipe();
}

HttpServer::~HttpServer() { ::close(m_idle_fd); }

void HttpServer::start() {
    assert(!m_is_started);
    m_is_started = true;

    m_accept_channel->set_read_cb(std::bind(&HttpServer::acceptor, this));
    m_accept_channel->register_read();

    m_thread_pool->start();
}

void HttpServer::acceptor() {

    while (true) {
        auto [ok, conn_sock] = m_listen_fd->accept();
        if (!ok) {
            /* Error: two many open file descriptors.
               Denial of service.
               Use idle fd to accept and then close it.
             */
            if (errno == EMFILE) {
                ::close(m_idle_fd);

                m_idle_fd = ::accept(m_listen_fd->get_fd(), nullptr, nullptr);
                ::close(m_idle_fd);

                m_idle_fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
                continue;
            } else
                return;
        }

        EventLoop *next_loop = m_thread_pool->get_next_loop();

        // suported http
        std::shared_ptr<HttpHandler> handler(
            new HttpHandler(next_loop, conn_sock));
        next_loop->queue_in_loop(
            std::bind(&EventLoop::add_http_connection, next_loop, handler));

        spdlog::info("New client connected, peer addr = {}, conn_fd = {}", 
                    conn_sock->peer_addr(), conn_sock->get_fd());
    }
}
} // namespace webserver