#include "event_loop_thread.h"

namespace webserver{
    EventLoopThread::EventLoopThread(const thread_init_cb& cb)
        : m_loop(nullptr), m_stopped(false), 
          m_thread(std::bind(&EventLoopThread::thread_func, this)),
          m_init_cb(cb){}

    EventLoopThread::~EventLoopThread(){
        m_loop = nullptr;
        m_stopped = true;
        m_thread.join();
    }

    EventLoop* EventLoopThread::start_loop(){
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while(m_loop == nullptr)
                m_cond.wait(lock);
        }
        return m_loop;
    }

    void EventLoopThread::thread_func(){
        EventLoop loop;
        if(m_init_cb){
            m_init_cb(&loop);
        }
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_loop = &loop;
            m_cond.notify_one();
        }
        loop.loop();
    }
}