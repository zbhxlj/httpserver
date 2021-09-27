#include "count_down_latch.h"
#include <mutex>

namespace webserver{
    void CountDownLatch::wait(){
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_count != 0) 
            m_cond.wait(lock);
    }

    void CountDownLatch::count_down(){
        std::unique_lock<std::mutex> lock(m_mutex);
        --m_count;
        if(m_count == 0)
            m_cond.notify_all();
    }

}