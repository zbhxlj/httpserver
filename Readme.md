# HttpServer Project

A webserver inspired by muduo developed using modern c++



# Introduction

该项目参考muduo实现了一个多线程静态的web服务器。HttpServer采用c++11编写，支持Get请求。

使用epoll ET边沿触发来提高实时性。HttpServer支持短连接、长连接，并采用timerfd实现了应用层心跳。

# Environment

Arch linux 5.14.14 

g++ 11.1.0

# Usage

- 编译

- `mkdir build && cd build && cmake .. && make -j8 && cd .. && mv build/web_server .` 

- `cd webbench && make`

  

- 运行

- 在config.yaml里设置ip 和 port，`./web_server.sh`

- `./webbench.sh`

# Architecture

该项目采用经典的reactor+NIO+thread pool+epoll ET模型设计而成。利用多核的优势，可显著提升实时响应能力。

# Performance Test

使用linux压测工具webbench，分别测试5k连接下，keepAlive和non-keepAlive请求。

测试负载为一小段HTML模板，内含一张图片。

60s同网段内测试，(keepalive和non-keepalive)响应能力均为0.30 million requests/min, 传输速度为 1.22M/s

# Perf Optimization

使用htop观察压测情况，发现每个线程CPU利用率只有30%左右，瓶颈在于IO

根据火焰图可以看到，IO和http处理占用了较多的时间