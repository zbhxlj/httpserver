#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include "noncopyable.h"
#include "epoll.h"
namespace webserver{

class Epoll;
class Channel;
extern thread_local pid_t tl_pid;
class EventLoop : public Noncopyable{
public:
    using functor = std::function<void()>;
    using channel_ptr = std::shared_ptr<Channel>;
    using channel_vector = std::vector<channel_ptr>;

    EventLoop(int timeout = 10000);
    ~EventLoop();
    void loop();
    void stop();
    void run_in_loop(functor && cb);
    void queue_in_loop(functor && cb);
    bool is_in_loop_thread() const { return m_pid == tl_pid; }
    static EventLoop* get_eventloop_of_cur_thread();
    void update_channel(const channel_ptr& channel);
    void remove_channel(const channel_ptr& channel);
private:
    void do_pending_functors();
    void clean_wakeup();
    void wakeup_from_poll();
    bool m_is_looping;
    std::atomic<bool> m_is_quit;
    pid_t m_pid;
    std::unique_ptr<Epoll> m_poller;
    int m_wakeup_fd;
    std::shared_ptr<Channel> m_wakeup_channel;
    channel_vector m_active_channels;
    bool m_is_calling_pending_cb;
    std::vector<functor> m_pending_cbs;
    std::mutex m_mutex;
    int m_poll_timeout;
};

}