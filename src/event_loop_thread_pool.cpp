#include <cassert>
#include "event_loop.h"
#include "event_loop_thread.h"
#include "event_loop_thread_pool.h"

namespace webserver{ 
    EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, int thread_nums)
    : m_base_loop(base_loop), m_is_started(false),
      m_thread_nums(thread_nums), m_turn(0){}

    // TODO delete thread
    EventLoopThreadPool::~EventLoopThreadPool(){}

    void EventLoopThreadPool::start(const thread_init_cb& cb){
        assert(!m_is_started);
        assert(m_base_loop->is_in_loop_thread());

        m_is_started = true;
        for(int i = 0; i < m_thread_nums; i++){
            EventLoopThread *t = new EventLoopThread(cb);
            m_threads.push_back(t);
            m_loops.push_back(t->start_loop());
        }

        if(m_thread_nums == 0 && cb) cb(m_base_loop);
    }

    EventLoop* EventLoopThreadPool::get_next_loop(){
        assert(m_base_loop->is_in_loop_thread());
        EventLoop* loop = m_base_loop;

        if(!m_loops.empty()){
            loop = m_loops[m_turn++];
            if(m_turn == m_thread_nums)
                m_turn = 0;
        }
        return loop;
    }
}