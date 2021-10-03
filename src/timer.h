#pragma once 

#include "event_loop.h"
#include "channel.h"
#include "noncopyable.h"

namespace webserver{

/*  A timer wraps the timerfd provided by kernel.
    Used in epoll.
*/
class Timer : public Noncopyable{

public:
    using cb = std::function<void()>;
    Timer(EventLoop* loop);
    ~Timer();
    void add_timer(int timeout);
    /* Reset timer.
       i.e. Reset timer of current timeout.
    */
    void reset();
    /* Repeatedly timed out.
    */
    void add_periodic_timer(int timeout);
    void set_timer_expire_cb(cb cb){
        m_expire_cb = cb;
    }
    /* A wrapper to deal with timed out events.
       Mainly clean readable events.
    */
    void read_cb();
private:
    EventLoop* m_loop;
    int m_timer_fd;
    std::shared_ptr<Channel> m_timer_channel;
    int m_timeout;
    cb m_expire_cb;
};

}