#pragma once 

#include <functional>
#include <sys/epoll.h>
#include <unistd.h>
#include <memory>
namespace webserver{

class EventLoop;
class Channel : public std::enable_shared_from_this<Channel>{
public:
    using event_cb = std::function<void()>;
    Channel(int fd, EventLoop* loop)
     : m_fd(fd), m_registered_events(0), m_triggered_events(0),
       m_loop(loop), m_read_cb(nullptr), m_write_cb(nullptr),
       m_close_cb(nullptr) {}
    ~Channel() { ::close(m_fd); }
    void dispatch_event();
    void set_read_cb(event_cb cb) { m_read_cb = cb; }
    void set_write_cb(event_cb cb) { m_write_cb = cb; }
    void set_close_cb(event_cb cb) { m_close_cb = cb; }
    int get_fd() const { return m_fd; }
    int get_register_events() const { return m_registered_events; }
    int get_triggered_events() const { return m_triggered_events; }
    void set_triggered_events(int events) { m_triggered_events = events; }
    bool is_none_event() const { return m_registered_events == NONE; }
    void register_read() {
        m_registered_events |= (READ | EPOLLET);
        update();
    }

    void register_write() {
        m_registered_events |= (WRITE | EPOLLET);
        update();
    }

    void unregister_read() { 
        m_registered_events &= ~READ; 
        update();
    }

    void unregister_write() { 
        m_registered_events &= ~WRITE; 
        update();
    }

    void unregister_all(){
        m_registered_events = NONE;
        update();
    }

    bool read_triggered() const { return m_triggered_events & READ; }
    bool write_triggered() const { return m_triggered_events & WRITE; }
    bool read_registered() const { return m_registered_events & READ; }
    bool write_registered() const { return m_registered_events & WRITE; }
    EventLoop* get_loop() const { return m_loop; }
private:
    void update();
    const int m_fd;
    int m_registered_events;
    int m_triggered_events;
    EventLoop* const m_loop;
    event_cb m_read_cb;
    event_cb m_write_cb;
    event_cb m_close_cb;
    static const int NONE = 0;
    static const int READ = EPOLLIN;
    static const int WRITE = EPOLLOUT;
};
struct ChannelHash{
	std::size_t operator()(const std::shared_ptr<Channel> &key) const
	{ 
		return std::hash<int>()(key->get_fd());
	}
};

struct ChannelCmp
{
	bool operator()(const std::shared_ptr<Channel> &lhs, 
	                const std::shared_ptr<Channel> &rhs) const
	{
		return lhs->get_fd() < rhs->get_fd();
	}
};

}