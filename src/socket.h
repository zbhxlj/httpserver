#pragma once

#include "inet_addr.h"

#define MAX_BUF_SIZE 4096
namespace webserver{

class TcpSocket{

public:
    TcpSocket() = default;
    explicit TcpSocket(std::string ip, uint16_t port);
    explicit TcpSocket(int sock_fd, InetAddr addr);
    // void bind();
    // void listen();
    void close();
    std::pair<bool, TcpSocket> accept();
    void shutdown(int type);
    void ignore_sigpipe();

    std::tuple<int, bool, std::string> recv();
    int send(std::string& buf);
    int get_fd() const { return m_socket_fd; }
private:
    int m_socket_fd;
    InetAddr m_addr;
};

}