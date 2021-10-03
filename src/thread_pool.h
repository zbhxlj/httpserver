#pragma once
#include <vector>
#include <memory>
#include "thread.h"
#include "concurrent_queue.h"
#include "noncopyable.h"
namespace webserver{

/* Thread pool implemented using lock-free multi-producer、multi-consumer queue.
*/
class ThreadPool : public Noncopyable{
public:
    using Task = std::function<void()>;
    ThreadPool(size_t thread_nums, size_t reserved_task_nums);
    void start();
    void stop();
    
    void add_task(Task task);

private:
    void consume_task();
    std::vector<std::unique_ptr<Thread>> m_threads;
    moodycamel::ConcurrentQueue<Task> m_task_queue;
    bool m_is_running;
    size_t m_thread_nums;
    size_t m_reserved_task_capacity;
    
};

}