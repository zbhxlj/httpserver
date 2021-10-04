#include "../src/event_loop.h"
#include "../src/event_loop_thread.h"
#include "../src/event_loop_thread_pool.h"
#include "../src/thread.h"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

namespace {
// void functor(std::string str){
//     spdlog::info("{}", str);
// }
// void thread_func_event_loop(){
//     spdlog::info("worker loop!");
//     EXPECT_EQ(webserver::EventLoop::get_eventloop_of_cur_thread(), nullptr);
//     webserver::EventLoop loop;
//     EXPECT_EQ(webserver::EventLoop::get_eventloop_of_cur_thread(), &loop);
//     loop.run_in_loop(std::bind(&functor, "Hello-World from sub thread"));
//     loop.loop();
// }
// TEST(EventLoopTest, plain){
//     spdlog::set_pattern("[%H:%M:%S %z] [thread %t] %v");
//     spdlog::info("main loop!");
//     EXPECT_EQ(webserver::EventLoop::get_eventloop_of_cur_thread(), nullptr);
//     webserver::EventLoop loop;
//     EXPECT_EQ(webserver::EventLoop::get_eventloop_of_cur_thread(), &loop);

//     webserver::Thread t1(thread_func_event_loop);
//     loop.run_in_loop(std::bind(&functor, "Hello-World from main thread"));
//     loop.loop();
// }

TEST(EventLoopThreadTest, plain) {
    spdlog::set_pattern("[%H:%M:%S %z] [thread %t] %v");
    webserver::EventLoopThread t1([](webserver::EventLoop *) {
        spdlog::info("Hello-World from event loop thread 1");
    });
    webserver::EventLoop *loop1 = t1.start_loop();
    webserver::EventLoopThread t2;
    webserver::EventLoop *loop2 = t2.start_loop();
    loop2->run_in_loop(
        []() { spdlog::info("Hello-World from event loop thread 2"); });
    EXPECT_NE(loop1, loop2);
    loop1->stop();
    loop2->stop();
}

TEST(EventLoopThreadPoolTest, plain) {
    spdlog::set_pattern("[%H:%M:%S %z] [thread %t] %v");
    webserver::EventLoop loop;
    {
        webserver::EventLoopThreadPool model(&loop, 0);
        model.start();
        EXPECT_EQ(model.get_next_loop(), &loop);
        EXPECT_EQ(model.get_next_loop(), &loop);
        EXPECT_EQ(model.get_next_loop(), &loop);
    }

    {
        webserver::EventLoopThreadPool model(&loop, 3);
        model.start();
        webserver::EventLoop *next_loop = model.get_next_loop();
        next_loop->run_in_loop([]() { spdlog::info("Hello-World"); });
        EXPECT_NE(next_loop, &loop);
        EXPECT_NE(next_loop, model.get_next_loop());
        EXPECT_NE(next_loop, model.get_next_loop());
        EXPECT_EQ(next_loop, model.get_next_loop());
    }
}
} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}