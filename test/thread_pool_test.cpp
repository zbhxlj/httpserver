#include "gtest/gtest.h"
#include "../src/thread_pool.h"

std::atomic<int> value{0};
TEST(ThreadPoolTest, plain){
    webserver::ThreadPool thread_pool(8, 1000);
    webserver::Thread add_task_thread(
        [&thread_pool](){
            int cnt = 1000;
            while(cnt--){
                thread_pool.add_task([](){ ++value; });
            }
        }
    );
    thread_pool.start();
    thread_pool.stop();
    EXPECT_EQ(value, 1000);
}
