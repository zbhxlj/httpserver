#include "event_loop.h"
#include "channel.h"
#include "epoll.h"
#include <sys/eventfd.h>
#include <spdlog/spdlog.h>

namespace webserver{ 
    thread_local EventLoop* tl_event_loop = nullptr;
    int create_event_fd(){
        int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(event_fd < 0){
            spdlog::error("create_event_fd failed: {}", event_fd);
        }
        return event_fd;
    }

    EventLoop::EventLoop(int timeout)
        :m_is_looping(false), m_is_quit(false), 
         m_pid(tl_pid), m_poller(std::make_unique<Epoll>()),
         m_wakeup_fd(create_event_fd()), m_wakeup_channel(std::make_shared<Channel>(m_wakeup_fd, this)),
         m_is_calling_pending_cb(false), m_poll_timeout(timeout),
         m_manager(std::make_unique<HttpManager>(this)){
        if(tl_event_loop != nullptr)
            spdlog::error("one thread only owns one event loop");
        tl_event_loop = this;
        m_wakeup_channel->set_read_cb(
            std::bind(&EventLoop::clean_wakeup, this));
        m_wakeup_channel->register_read();
    }
    
    EventLoop::~EventLoop(){
        ::close(m_wakeup_fd);
        tl_event_loop = nullptr;
    }

    EventLoop* EventLoop::get_eventloop_of_cur_thread (){
        return tl_event_loop;
    }
    void EventLoop::loop(){
        assert(!m_is_looping);
        assert(is_in_loop_thread());
       
        m_is_looping = true;
        channel_vector active_channels;
        
        while(!m_is_quit){
            active_channels.clear();
            active_channels = m_poller->poll(m_poll_timeout);

            for(auto& channel : active_channels){
                // 处理读写事件
                channel->dispatch_event();
                // 处理业务逻辑
                m_manager->handler(channel);
            }

            do_pending_functors();
        }
        m_is_looping = false;
    }

    void EventLoop::stop(){
        m_is_quit = true;
        if(!is_in_loop_thread()) 
            wakeup_from_poll();
    }

    void EventLoop::wakeup_from_poll(){
        uint64_t tickle = 1;
        ssize_t write_bytes = ::write(m_wakeup_fd, &tickle, sizeof(tickle));
        if(write_bytes != sizeof(tickle)){
            spdlog::info("write_bytes = {}", write_bytes);
            spdlog::warn("Not successfully write to wakeup_from_poll!");
        }
    }

    void EventLoop::clean_wakeup(){
        uint64_t tickle;
        ssize_t received_bytes = read(m_wakeup_fd, &tickle, sizeof(tickle));
        if(received_bytes != sizeof(tickle))   
            spdlog::warn("Not successfully read to clean wakeup!");
    }

    void EventLoop::run_in_loop(functor&& cb){
        if(is_in_loop_thread()){
            cb();
        }else {
            queue_in_loop(std::move(cb));
        }
    }

    void EventLoop::queue_in_loop(functor&& cb){
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_pending_cbs.emplace_back(cb);
        }
        if(!is_in_loop_thread() || m_is_calling_pending_cb)
            wakeup_from_poll();
    }

    void EventLoop::do_pending_functors(){
        std::vector<functor> pending_functors;
        m_is_calling_pending_cb = true;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            pending_functors.swap(m_pending_cbs);
        }
        for(auto& f : pending_functors){
            f();
        }
        m_is_calling_pending_cb = false;
    }

    void EventLoop::update_channel(const channel_ptr& channel){
        assert(is_in_loop_thread());
        assert(channel->get_loop() == this);
        m_poller->update_channel(channel);
    }

    void EventLoop::remove_channel(channel_ptr& channel){
        assert(is_in_loop_thread());
        assert(channel->get_loop() == this);
        m_poller->remove_channel(channel);
        m_manager->del_http_connection(channel);
    }

    void EventLoop::add_http_connection(http_handler_ptr handler){
	    m_manager->add_http_connection(handler);
    }

    void EventLoop::flush_keep_alive(channel_ptr &channel, HttpManager::timer_node &node){
        m_manager -> flush_keep_alive(channel, node);
    }

}