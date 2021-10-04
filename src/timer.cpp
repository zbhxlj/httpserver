#include "timer.h"
#include <cassert>
#include <spdlog/spdlog.h>
#include <strings.h>
#include <sys/timerfd.h>

namespace webserver {
Timer::Timer(EventLoop *loop)
    : m_loop(loop),
      m_timer_fd(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      m_timer_channel(std::make_shared<Channel>(
          std::make_shared<TcpSocket>(m_timer_fd, InetAddr()), loop)),
      m_timeout(0), m_expire_cb(nullptr) {
    assert(m_loop != nullptr);
    assert(m_timer_fd > 0);
    m_timer_channel->set_read_cb(std::bind(&Timer::read_cb, this));
}

Timer::~Timer() {
    ::close(m_timer_fd);
    m_expire_cb = nullptr;
}

void Timer::add_timer(int timeout) {
    m_timeout = timeout;
    itimerspec new_timeout;
    bzero(&new_timeout, sizeof(new_timeout));
    new_timeout.it_value.tv_sec = timeout;

    int ret = ::timerfd_settime(m_timer_fd, 0, &new_timeout, nullptr);
    assert(ret >= 0);

    if (!m_timer_channel->read_registered()) {
        m_timer_channel->register_read();
    }
}

void Timer::reset() {
    assert(m_timeout > 0);
    add_timer(m_timeout);
}

void Timer::add_periodic_timer(int timeout) {
    m_timeout = timeout;

    itimerspec new_timeout;
    bzero(&new_timeout, sizeof(new_timeout));
    new_timeout.it_value.tv_sec = timeout;
    new_timeout.it_interval.tv_sec = timeout;

    int ret = ::timerfd_settime(m_timer_fd, 0, &new_timeout, nullptr);
    assert(ret >= 0);

    if (!m_timer_channel->read_registered()) {
        m_timer_channel->register_read();
    }
}

void Timer::read_cb() {
    uint64_t one;
    int read_bytes = ::read(m_timer_fd, &one, sizeof(one));
    if (read_bytes != sizeof(one)) {
        spdlog::error("Not successfully read from timer_fd");
        abort();
    }

    assert(m_expire_cb != nullptr);
    m_expire_cb();
}
} // namespace webserver