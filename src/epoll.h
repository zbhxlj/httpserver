#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>
#include "noncopyable.h"

namespace webserver{ 

class Channel;
class Epoll : public Noncopyable {

public:
    using channel_ptr = std::shared_ptr<Channel>;
    using channel_vector = std::vector<channel_ptr>;
    Epoll();
    ~Epoll();
    channel_vector poll(int timeout);

    void update_channel(const channel_ptr& channel);
    void remove_channel(const channel_ptr& channel);
private:
    void update_event(const channel_ptr& channel, int op);
    const int m_init_events_size = 16;
    int m_epfd;
    std::vector<epoll_event> m_events;
    std::unordered_map<int, channel_ptr> m_channel_map;
};

}