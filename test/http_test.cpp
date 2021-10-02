#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "../src/timer.h"

TEST(TimerTest, plain){
    webserver::EventLoop loop;
    webserver::Timer timer(&loop);
    
    int timeout = 3;
    timer.set_timer_expire_cb([timeout](){ 
        spdlog::info("Timer expired after {}", timeout);
    });

    timer.add_periodic_timer(timeout);
    loop.loop();
}

int main(int argc, char **argv){ 
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}