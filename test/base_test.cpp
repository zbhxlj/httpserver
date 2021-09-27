#include <gtest/gtest.h>
#include "../src/config.h"
#include "../src/thread.h"
#include "../src/thread_pool.h"

namespace {
    TEST(ConfigParserTest, plain){
        EXPECT_EQ(
            webserver::WebConfig("127.0.0.1", 8080, "/home/zbh/project/csWeb", 8),  
            webserver::load_config("/home/zbh/project/csWeb/config/config.yaml")
        );
    }

    std::atomic<int> value1{0};
    TEST(ThreadPoolTest, plain){
        webserver::ThreadPool thread_pool(8, 1000);
        webserver::Thread add_task_thread(
            [&thread_pool](){
                int cnt = 1000;
                while(cnt--){
                    thread_pool.add_task([](){ ++value1; });
                }
            }
        );
        thread_pool.start();
        thread_pool.stop();
        EXPECT_EQ(value1, 1000);
    }

    

    int value2 = 0;
    std::mutex mutex;
    void thread_func(){
        std::unique_lock<std::mutex> lock(mutex);
        ++value2;
    }
    TEST(ThreadTest, plain){
        std::vector<std::unique_ptr<webserver::Thread>> threads;
        const int cnt = 100;
        
        for(int i = 0; i < cnt; i++){
            threads.emplace_back(std::make_unique<webserver::Thread>(thread_func));
        }

        for(const auto& thread : threads)
            thread->join();

        EXPECT_EQ(value2, cnt);
    }
}

int main(int argc, char **argv){ 
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}