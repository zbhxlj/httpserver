#pragma once

#include <condition_variable>
#include "noncopyable.h"
namespace webserver{
class CountDownLatch : public Noncopyable{
public:
    CountDownLatch(size_t count): m_count(count){}

    void wait();
    void count_down();
private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    size_t m_count;
};

}