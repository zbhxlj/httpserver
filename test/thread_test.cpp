#include "gtest/gtest.h"
#include <mutex>
#include <memory>
#include <vector>
#include "../src/thread.h"

int value = 0;
std::mutex mutex;
void thread_func(){
    std::unique_lock<std::mutex> lock(mutex);
    ++value;
}
TEST(ThreadTest, plain){
    std::vector<std::shared_ptr<webserver::Thread>> threads;
    const int cnt = 100;
    
    for(int i = 0; i < cnt; i++){
        threads.emplace_back(std::make_shared<webserver::Thread>(thread_func));
    }

    for(const auto& thread : threads)
        thread->join();

    EXPECT_EQ(value, cnt);
}