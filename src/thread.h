#pragma once

#include <string>
#include <functional>
#include "noncopyable.h"
#include "count_down_latch.h"

namespace webserver{

class Thread : public Noncopyable{

public:
    using thread_func = std::function<void()>;

    Thread(thread_func func, std::string name = std::string());


    static void* start(void* args);
    void join();
    bool is_started() const{ return m_is_started; };
    pid_t get_tid() const{ return m_pid; };
    void set_thread_name(std::string name);

private:
    pthread_t m_pthread_id;
    pid_t m_pid;
    thread_func m_thread_func;
    std::string m_thread_name;
    bool  m_is_started;
    bool m_is_joined;
    CountDownLatch m_latch;
};

}