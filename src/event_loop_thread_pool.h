#pragma once

#include "event_loop_thread.h"
#include <vector>
namespace webserver{

class EventLoopThreadPool{

public :
    using thread_init_cb = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* base_loop, int thread_nums = 0);
    ~EventLoopThreadPool();
    void start(const thread_init_cb& cb = thread_init_cb());
    EventLoop* get_next_loop();
private:
    EventLoop* m_base_loop;
    bool m_is_started;
    int m_thread_nums;
    int m_turn;
    std::vector<EventLoopThread*> m_threads;
    std::vector<EventLoop*> m_loops;
};

}