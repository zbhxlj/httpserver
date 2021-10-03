#pragma once

#include <string>
#include <functional>
#include "noncopyable.h"
#include "count_down_latch.h"

namespace webserver{

/* Thread is a wrapper of pthread.
*/
class Thread : public Noncopyable{

public:
    using thread_func = std::function<void()>;

    Thread(thread_func func);
    /* Due to pthread_create(void* func(void*), void* args),
       we must pass it a static member function, 
       as non_static member function has a implicit "this" pointer.
       Through passing "this", we can still access its non-static data.
    */
    static void* start(void* args);
    void join();
    bool is_started() const{ return m_is_started; };
    /* Use pid to represent a thread.
       Because it is globally unique in a short time. 
    */
    pid_t get_tid() const{ return m_pid; };

private:
    pthread_t m_pthread_id;
    pid_t m_pid;
    thread_func m_thread_func;
    bool  m_is_started;
    bool m_is_joined;
    CountDownLatch m_latch;
};

}