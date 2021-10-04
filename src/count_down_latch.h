#pragma once

#include "noncopyable.h"
#include <condition_variable>
namespace webserver {

/* Thread Syncronization
   1. A parent thread waits for a pool of threads to complete/create.
   2. All subthreads wait for one another to create.
*/
class CountDownLatch : public Noncopyable {
  public:
    /* @param The thread number we will be waiting for.
     */
    CountDownLatch(size_t count) : m_count(count) {}
    /* Wait for one another.
     */
    void wait();
    /* Current thread has been created/completed.
     */
    void count_down();

  private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    size_t m_count;
};

} // namespace webserver