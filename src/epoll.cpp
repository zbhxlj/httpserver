#include "epoll.h"
#include "channel.h"
#include <spdlog/spdlog.h>
#include <strings.h>
#include <unistd.h>
namespace webserver {
Epoll::Epoll() : m_epfd(::epoll_create(1000)), m_events(m_init_events_size) {}
Epoll::~Epoll() { ::close(m_epfd); }
Epoll::channel_vector Epoll::poll(int timeout) {
    int triggered_nums =
        ::epoll_wait(m_epfd, m_events.data(), m_events.size(), timeout);

    if (triggered_nums < 0) {
        spdlog::error("epoll error!");
        abort();
    }

    channel_vector active_channels;
    active_channels.reserve(triggered_nums);

    for (int i = 0; i < triggered_nums; i++) {
        const auto &event = m_events[i];
        int fd = event.data.fd;
        m_channel_map[fd]->set_triggered_events(event.events);
        active_channels.push_back(m_channel_map[fd]);
    }

    if (triggered_nums == static_cast<int>(m_events.size()))
        m_events.resize(triggered_nums * 2);

    return active_channels;
}

void Epoll::update_channel(const channel_ptr &channel) {
    int fd = channel->get_fd();
    if (m_channel_map.find(fd) == m_channel_map.end()) {
        update_event(channel, EPOLL_CTL_ADD);
        m_channel_map.insert(std::make_pair(fd, channel));
    } else {
        update_event(channel, EPOLL_CTL_MOD);
    }
}

void Epoll::remove_channel(const channel_ptr &channel) {
    int fd = channel->get_fd();
    if (m_channel_map.find(fd) == m_channel_map.end()) {
        spdlog::error("remove channel failed! channel not exist!");
        abort();
    }
    update_event(channel, EPOLL_CTL_DEL);
    m_channel_map.erase(fd);
}

void Epoll::update_event(const channel_ptr &channel, int op) {
    int fd = channel->get_fd();

    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = fd;
    event.events = channel->get_register_events();

    if (::epoll_ctl(m_epfd, op, fd, &event) < 0)
        spdlog::error("Failed to epoll_ctl!");
}
} // namespace webserver