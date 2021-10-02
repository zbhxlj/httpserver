#pragma once 

#include "event_loop.h"
#include "channel.h"

namespace webserver{

class Timer{

public:
    using cb = std::function<void()>;
    Timer(EventLoop* loop);
    ~Timer();
    void add_timer(int timeout);
    void reset();
    void add_periodic_timer(int timeout);
    void set_timer_expire_cb(cb cb){
        m_expire_cb = cb;
    }
    void read_cb();
private:
    EventLoop* m_loop;
    int m_timer_fd;
    std::shared_ptr<Channel> m_timer_channel;
    int m_timeout;
    cb m_expire_cb;
};

}