#include "thread_pool.h"
#include "spdlog/spdlog.h"
#include "iostream"

namespace webserver{
    ThreadPool::ThreadPool(size_t pool_size, size_t reserved_task_capacity)
        : m_task_queue(reserved_task_capacity),m_is_running(false), 
          m_thread_nums(pool_size), m_reserved_task_capacity(reserved_task_capacity){
            m_threads.reserve(pool_size);
        }
    
    void ThreadPool::start(){
        m_is_running = true;
        for(size_t i = 0; i < m_thread_nums; i++){
            m_threads.emplace_back(std::make_unique<Thread>(
                [this](){ this->consume_task(); })
                );
        }
        spdlog::info("Start thread pool!");
    }

    void ThreadPool::stop(){
        m_is_running = false;
        for(auto& thread : m_threads){
            thread->join();
        }
        spdlog::info("Stop thread pool!");
    }

    void ThreadPool::add_task(Task task){
        bool ok = false;
        while(!ok){
            ok = m_task_queue.try_enqueue(std::move(task));
        }
    }

    void ThreadPool::consume_task(){
        Task task;
        bool found = false;
        while(m_is_running || found){
            found = m_task_queue.try_dequeue(task);
            if(found){
                task();
            }
        }
    }
}