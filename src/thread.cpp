#include "thread.h"
#include "spdlog/spdlog.h"
#include <memory>
#include <pthread.h>

namespace webserver {
/*  Current thread's pid and thread instance.
 */
thread_local pid_t tl_pid;
thread_local Thread *tl_thread;
Thread::Thread(thread_func func)
    : m_pthread_id(0), m_pid(0), m_thread_func(std::move(func)),
      m_is_started(false), m_is_joined(false), m_latch(1) {
    int ret = pthread_create(&m_pthread_id, nullptr, &start, this);
    if (ret != 0) {
        spdlog::error("Failed to create thread. pthread_create failed!");
        abort();
    }
    m_latch.wait();
}

void *Thread::start(void *args) {
    Thread *thread = static_cast<Thread *>(args);

    thread->m_is_started = true;
    thread->m_latch.count_down();
    thread->m_pid = ::syscall(SYS_gettid);

    tl_pid = thread->m_pid;
    tl_thread = thread;

    spdlog::info("Thread {} created successfully", thread->m_pid);
    thread->m_thread_func();

    return nullptr;
}

void Thread::join() {
    if (!m_is_joined) {
        m_is_joined = true;
        pthread_join(m_pthread_id, nullptr);
    }
}
} // namespace webserver