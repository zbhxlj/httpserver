#include "http_manager.h"
#include "http_connection.h"
#include "http_handler.h"
#include "timer.h"
#include <cassert>
#include <spdlog/spdlog.h>
#include <strings.h>
#include <sys/time.h>

namespace webserver {

HttpManager::HttpManager(EventLoop *loop)
    : m_loop(loop), m_timer(std::make_unique<Timer>(loop)) {
    m_timer->set_timer_expire_cb(
        std::bind(&HttpManager::handle_expire_event, this));
    m_timer->add_periodic_timer(3);
}

HttpManager::~HttpManager() {}

void HttpManager::handler(channel_ptr &channel) {
    if (channel->read_registered() &&
        m_http_map.find(channel) != m_http_map.end()) {
        spdlog::debug("Handle new http request");
        m_http_map[channel]->handle_http_request();
    }
}

void HttpManager::add_http_connection(http_handler_ptr &handler) {
    auto &channel = handler->m_connection->get_channel();
    m_http_map.insert({channel, handler});
    handler->new_connection();
    spdlog::info("Establish new http connection, peer's address = {}",
                 handler->m_conn_fd->peer_addr());
}

void HttpManager::del_http_connection(channel_ptr &channel) {
    m_http_map.erase(channel);
    if (m_keep_alive_set.find(channel) != m_keep_alive_set.end()) {
        m_keep_alive_set.erase(channel);
    }
    spdlog::info("Delete http connection, peer's address = {}",
                 channel->peer_addr());
}

void HttpManager::flush_keep_alive(const channel_ptr &channel,
                                   timer_node &node) {
    timeval time;
    bzero(&time, sizeof(time));

    int ret = gettimeofday(&time, NULL);
    assert(ret >= 0);
    time.tv_sec += 120;

    if (m_keep_alive_set.find(channel) != m_keep_alive_set.end()) {
        m_keep_alive_list.erase(node);
    } else {
        m_keep_alive_set.insert(channel);
    }

    timer_node it =
        m_keep_alive_list.insert(m_keep_alive_list.end(), {channel, time});
    node = it;
}

void HttpManager::handle_expire_event() {
    timeval time;
    bzero(&time, sizeof(time));

    int ret = gettimeofday(&time, nullptr);
    assert(ret >= 0);

    for (auto it = m_keep_alive_list.begin(); it != m_keep_alive_list.end();
         it++) {
        if (it->second.tv_sec > time.tv_sec) {
            m_keep_alive_list.erase(m_keep_alive_list.begin(), it);
            break;
        }
        if (m_keep_alive_set.find(it->first) != m_keep_alive_set.end()) {
            m_keep_alive_set.erase(it->first);
            auto &handler = m_http_map[it->first];
            handler->m_connection->set_state(HttpConnection::DisConnected);
            handler->m_connection->handle_close();
        }
    }
}
} // namespace webserver
