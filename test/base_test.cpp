#include <gtest/gtest.h>
#include "spdlog/spdlog.h"
#include "../src/config.h"
#include "../src/thread.h"
#include "../src/thread_pool.h"
#include "../src/socket.h"
#include <sys/epoll.h>

namespace {
    // TEST(ConfigParserTest, plain){
    //     EXPECT_EQ(
    //         webserver::WebConfig("127.0.0.1", 8080, "/home/zbh/project/csWeb", 8),  
    //         webserver::load_config("/home/zbh/project/csWeb/config/config.yaml")
    //     );
    // }

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

    // TEST(SocketAndAddrTest, plain){
    //     webserver::Thread client(
    //         [](){
    //             int client_fd = socket(AF_INET, SOCK_STREAM, 0);
                
    //             sockaddr_in server_addr;
    //             server_addr.sin_family = AF_INET;
    //             const char* ip_addr = "127.0.0.1";
    //             inet_pton(AF_INET, ip_addr, &server_addr.sin_addr.s_addr);
    //             server_addr.sin_port = htons(8080);

    //             int ret = connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    //             if(ret != 0)
    //                 spdlog::error("Failed to connect!");
                
    //             const std::string content = "Hello, world!";
    //             auto bytes = write(client_fd, content.c_str(), content.size());
    //             if(bytes != static_cast<long int>(content.size()))
    //                 spdlog::error("Failed to write all bytes!");
    //         }
    //     );

    //     webserver::TcpSocket listen_fd("127.0.0.1", 8080);
    //     webserver::TcpSocket client_sock;
    //     while(true){
    //         auto [connected, client_sock] = listen_fd.accept();
    //         if(connected) break;
    //     }

    //     int epfd = ::epoll_create(10);
    //     epoll_event event;
    //     event.events = EPOLLIN | EPOLLET;
    //     event.data.fd = client_sock.get_fd();
    //     ::epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock.get_fd(), &event);

    //     ::epoll_wait(epfd, &event, 1, -1);

    //     auto [total_bytes, is_finished, msg] = client_sock.recv();
    //     if(total_bytes != 0){
    //         EXPECT_EQ(msg, "Hello, World!");
    //         EXPECT_EQ(is_finished, true);
    //         client_sock.close();
    //     }else {
    //         spdlog::info("Received nothing from server");
    //     }
    // }
}

int main(int argc, char **argv){ 
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}