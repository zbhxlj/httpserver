#include "thread.h"
#include "spdlog/spdlog.h"
#include <pthread.h>
#include <memory>

namespace webserver{
    static thread_local pid_t tl_pid;
    static thread_local Thread* tl_thread;
    Thread::Thread(thread_func func, std::string name)
        :m_thread_func(std::move(func)),m_thread_name(std::move(name)), m_latch(1){
        int ret = pthread_create(&m_pthread_id, nullptr, &start, this);
        if(ret != 0){
            spdlog::error("Failed to create threadpthread_create failed!");
            abort();
        }
        m_latch.wait();
    }

    void* Thread::start(void* args){
        Thread* thread = static_cast<Thread*>(args);
        
        thread->m_is_started = true;
        thread->m_latch.count_down();    
        thread->m_pid = ::syscall(SYS_gettid);

        tl_pid = thread->m_pid;
        tl_thread = thread;

        spdlog::info("Thread {} created successfully", thread->m_pid);
        thread->m_thread_func();

        return nullptr;
    }

    void Thread::join(){
        if(!m_is_joined){
            m_is_joined = true;
            pthread_join(m_pthread_id, nullptr);
        }
    }

    void Thread::set_thread_name(std::string name){
        m_thread_name = std::move(name);
    }
}