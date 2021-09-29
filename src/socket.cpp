#include "socket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <spdlog/spdlog.h>
#include <utility>
#include <cstdio>

namespace webserver{
    TcpSocket::TcpSocket(std::string ip, uint16_t port): m_addr(ip, port){
        m_socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        int optval = 1;
        ::fcntl(m_socket_fd, F_SETFL, O_NONBLOCK);
        ::setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if(m_socket_fd < 0){
            spdlog::error("Failed to create socket!");
            abort();
        }
        if(::bind(m_socket_fd, (struct sockaddr*)&m_addr.get_addr(), sizeof(m_addr.get_addr())) < 0){
            spdlog::error("Failed to bind ip address!");
            strerror(errno);
            abort();
        }
        if(::listen(m_socket_fd, 1000) < 0){
            spdlog::error("Failed to listen!");
            abort();
        }
    }

    TcpSocket::TcpSocket(int sock_fd, InetAddr addr)
        :m_socket_fd(sock_fd), m_addr(std::move(addr)){}

    std::pair<bool, TcpSocket> TcpSocket::accept(){
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int conn_fd = ::accept4(m_socket_fd, (sockaddr*)&client_addr,&addr_len, SOCK_NONBLOCK);
        auto conn_sock = TcpSocket(conn_fd, InetAddr(std::move(client_addr)));
        if(conn_fd == -1) return {false, conn_sock};
        else return {true, conn_sock};
    }

    void TcpSocket::close(){
        int ret = ::close(m_socket_fd);
        if(ret < 0)
            spdlog::warn("Unclean close! socket fd = {}", m_socket_fd);
    }

    void TcpSocket::shutdown(int type){
        int ret = ::shutdown(m_socket_fd, type);
        if(ret < 0)
            spdlog::warn("Unclean shutdown! socket fd = {}", m_socket_fd);
    }

    void TcpSocket::ignore_sigpipe(){
        ::signal(SIGPIPE, SIG_IGN);
    }

    std::tuple<int, bool, std::string> TcpSocket::recv(){
        char buffer[MAX_BUF_SIZE];
        int read_bytes = 0;
        int recv_bytes = 0;
        std::string read_buf;
        
        while(true) {
            if((read_bytes = ::recv(m_socket_fd, buffer, MAX_BUF_SIZE, 0)) <= 0){
                spdlog::info("read_bytes = {}, errno = {}", read_bytes, strerror(errno));
                if(read_bytes == 0){
                    return { recv_bytes, true, read_buf };
                }
                if(errno == EINTR) continue;
                if(errno == EAGAIN || errno == EWOULDBLOCK) return { recv_bytes, false, read_buf };
                return { recv_bytes, false, read_buf };
            }
            recv_bytes += read_bytes;
            read_buf += std::string(buffer, read_bytes);
        }
        spdlog::error("Never reach here!");
        return {-1, false, std::string()};
    }

    int TcpSocket::send(std::string& buf){
        int write_bytes;
        int send_bytes;
        int buffer_bytes = buf.size();
        const char* ptr = buf.data();

        while(send_bytes < buffer_bytes){
            write_bytes = ::send(m_socket_fd, ptr + send_bytes, buffer_bytes - send_bytes, 0);
            if(write_bytes < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                else if(errno == EINTR) continue;
 
                buf.clear();
                spdlog::warn("Write occured error {}", ::strerror(errno));
                return -1;
            }
            send_bytes += write_bytes;
        }

        if(write_bytes == send_bytes){
            buf.clear();
        }else {
            buf = buf.substr(send_bytes);
        }
        return send_bytes;
    }
}