#pragma once
#include "thread.h"
#include <mutex>
#include <condition_variable>
#include "noncopyable.h"
namespace webserver{

class EventLoop;
/* Thread containing a eventloop.
   Used by eventloop_thread_pool.
*/
class EventLoopThread : public Noncopyable{

public:
    /* A section of code, when we want to do some other things.
    */
    using thread_init_cb = std::function<void(EventLoop*)>;
    EventLoopThread(const thread_init_cb& cb = thread_init_cb());
    ~EventLoopThread();
    EventLoop* start_loop();
private:
    void thread_func();
    EventLoop* m_loop;
    bool m_stopped;
    Thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    thread_init_cb m_init_cb;

};

}