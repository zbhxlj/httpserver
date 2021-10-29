#!/bin/bash

# non keep-alive 5000并发访问60秒 Http1.0
WebBench/webbench -c 5000 -t 60 -1 --get http://your_ip_addr:8080/ > 5k-non-keepalive.log

# keep-alive 5000并发访问60秒 HTTP1.1
WebBench/webbench -c 5000 -t 60 -2 --get http://your_ip_addr:8080/ > 5k-keepalive.log

