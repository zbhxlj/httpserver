#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>
#include "noncopyable.h"

namespace webserver{ 

class Channel;
/* A wrapper of epollfd.
   A mock of kernel's epollfd.
   One loop one thread one epoll.
*/
class Epoll : public Noncopyable {

public:
    using channel_ptr = std::shared_ptr<Channel>;
    using channel_vector = std::vector<channel_ptr>;
    Epoll();
    ~Epoll();
    /* A wrapper of epoll_wait.
       Set triggered events according to epoll_wait.
    */
    channel_vector poll(int timeout);
    /* A wrapper of epoll_ctl.
    */
    void update_channel(const channel_ptr& channel);
    void remove_channel(const channel_ptr& channel);
private:
    /* Internal used by update_channel.
    */
    void update_event(const channel_ptr& channel, int op);
    const int m_init_events_size = 16;
    int m_epfd;
    /* Triggered events returned by epoll_wait.
       Same role as the kernel's ready list.
    */
    std::vector<epoll_event> m_events;
    /* Registered channel saved in a RB-tree.
       Same role as the kernel's RB-tree.
    */ 
    std::unordered_map<int, channel_ptr> m_channel_map;
};

}