#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "../src/timer.h"
#include "../src/http_server.h"
#include "../src/event_loop_thread_pool.h"
#include "../src/inet_addr.h"

// TEST(TimerTest, plain){
//     webserver::EventLoop loop;
//     webserver::Timer timer(&loop);
    
//     int timeout = 3;
//     timer.set_timer_expire_cb([timeout](){ 
//         spdlog::info("Timer expired after {}", timeout);
//     });

//     timer.add_periodic_timer(timeout);
//     loop.loop();
// }

TEST(HttpServerTest, plain){
	webserver::EventLoop mainLoop;
	
	webserver::HttpServer server(&mainLoop, "127.0.0.1", 12345, 3);
	server.start();
	
	mainLoop.loop();
}

int main(int argc, char **argv){ 
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}