#include "channel.h"
#include "event_loop.h"

namespace webserver {
/* Take error as readable and writable events.
   Inspired by redis and libevent.
*/
void Channel::dispatch_event() {
    if (m_triggered_events & EPOLLERR) {
        m_triggered_events |= ((READ | WRITE) & m_registered_events);
    }
    if ((m_triggered_events & EPOLLHUP) && !(m_triggered_events & EPOLLRDHUP)) {
        m_triggered_events |= ((READ | WRITE) & m_registered_events);
    }
    if (m_triggered_events & READ)
        m_read_cb();
    if (m_triggered_events & WRITE)
        m_write_cb();
    if (m_triggered_events & EPOLLRDHUP)
        m_close_cb();
}

void Channel::update() { m_loop->update_channel(shared_from_this()); }
} // namespace webserver