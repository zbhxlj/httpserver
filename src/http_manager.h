#pragma once

#include <unordered_map>
#include <unordered_set>
#include <list>
#include "channel.h"
#include "noncopyable.h"
namespace webserver{

class HttpHandler;
class EventLoop;
class Channel;
class Timer;
class HttpManager : public Noncopyable{

public:
    using channel_ptr = std::shared_ptr<Channel>;
    using http_handler_ptr = std::shared_ptr<HttpHandler>;
    using entry = std::pair<channel_ptr, timeval>;
    using timer_node = std::list<entry>::iterator;

    HttpManager(EventLoop* loop);
    ~HttpManager();
    void handler(channel_ptr& channel);
    void add_http_connection(http_handler_ptr& http_handler);
    void del_http_connection(channel_ptr& channel);
    void flush_keep_alive(const channel_ptr& channel, HttpManager::timer_node &node);
    void handle_expire_event();

private:
    EventLoop* m_loop;
    std::unique_ptr<Timer> m_timer;
    std::unordered_map<channel_ptr, http_handler_ptr, ChannelHash> m_http_map;
    std::list<entry> m_keep_alive_list;
    std::unordered_set<channel_ptr, ChannelHash> m_keep_alive_set;
};

}