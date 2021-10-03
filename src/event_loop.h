#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include "noncopyable.h"
#include "http_manager.h"
namespace webserver{

class Epoll;
class Channel;
class HttpHandler;
extern thread_local pid_t tl_pid;
/*  Abstract of loop in worker thread.
*/
class EventLoop : public Noncopyable{
public:
    using functor = std::function<void()>;
    using channel_ptr = std::shared_ptr<Channel>;
    using channel_vector = std::vector<channel_ptr>;
    using http_handler_ptr = std::shared_ptr<HttpHandler>;
    EventLoop(int timeout = 10000);
    ~EventLoop();
    void loop();
    void stop();
    /* Schedule callback to run in curent loop. 
       Called in the same thread.
    */
    void run_in_loop(functor && cb);
    /* Schedule callback to run in current loop.
       Called in othre thread.
    */
    void queue_in_loop(functor && cb);
    bool is_in_loop_thread() const { return m_pid == tl_pid; }
    static EventLoop* get_eventloop_of_cur_thread();
    /* Delegate Epoll to update.
    */
    void update_channel(const channel_ptr& channel);
    void remove_channel(channel_ptr& channel);

    // support http
    void add_http_connection(http_handler_ptr handler);
    void flush_keep_alive(channel_ptr& channel, HttpManager::timer_node& node);
private:
    void do_pending_functors();
    void clean_wakeup();
    void wakeup_from_poll();
    bool m_is_looping;
    bool m_is_quit;
    pid_t m_pid;
    std::unique_ptr<Epoll> m_poller;
    /* Eventfd provided by kernel used in epoll.
    */
    int m_wakeup_fd;
    std::shared_ptr<Channel> m_wakeup_channel;
    channel_vector m_active_channels;
    bool m_is_calling_pending_cb;
    std::vector<functor> m_pending_cbs;
    std::mutex m_mutex;
    int m_poll_timeout;
    // support http
    std::unique_ptr<HttpManager> m_manager;
};

}
