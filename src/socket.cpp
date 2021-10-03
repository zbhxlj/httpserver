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

    TcpSocket::~TcpSocket(){
        close();
    }
    std::pair<bool, TcpSocket::socket_ptr> TcpSocket::accept(){
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int conn_fd = ::accept4(m_socket_fd, (sockaddr*)&client_addr,&addr_len, SOCK_NONBLOCK);
        auto conn_sock = std::make_shared<TcpSocket>(conn_fd, InetAddr(std::move(client_addr)));
        
        if(conn_fd == -1) return {false, conn_sock};
        else return {true, conn_sock};
    }

    void TcpSocket::close(){
        // int ret = ::close(m_socket_fd);
        // if(ret < 0)
            // spdlog::warn("Unclean close! socket fd = {} errno = {}", m_socket_fd, ::strerror(errno));
    }

    void TcpSocket::shutdown(int type){
        int ret = ::shutdown(m_socket_fd, type);
        if(ret < 0)
            spdlog::warn("Unclean shutdown! socket fd = {}", m_socket_fd);
    }

    void TcpSocket::ignore_sigpipe(){
        ::signal(SIGPIPE, SIG_IGN);
    }

    std::pair<int, bool> TcpSocket::recv(std::string& buf){
        char buffer[MAX_BUF_SIZE];
        int read_bytes = 0;
        int total_recv_bytes = 0;
        
        while(true) {
            if((read_bytes = ::recv(m_socket_fd, buffer, MAX_BUF_SIZE, 0)) <= 0){
                if(errno == EINTR) continue;
                if(errno == EAGAIN || errno == EWOULDBLOCK) return { total_recv_bytes, false };
                if(read_bytes == 0){
                    return { total_recv_bytes, true };
                }
                spdlog::warn("Recv error occured : {} ", ::strerror(errno));
                return { -1, false };
            }
            total_recv_bytes += read_bytes;
            buf += std::string(buffer, read_bytes);
        }
        spdlog::error("Never reach here!");
        abort();
        return { -1, false };
    }

    int TcpSocket::send(std::string& buf){
        int send_bytes;
        int already_send_bytes = 0;
        int to_send_bytes = buf.size();
        const char* ptr = buf.data();

        while(already_send_bytes < to_send_bytes){
            send_bytes = ::send(m_socket_fd, ptr + already_send_bytes, to_send_bytes - already_send_bytes, 0);
            if(send_bytes <= 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                if(errno == EINTR) continue;
 
                buf.clear();
                spdlog::warn("Write error occured: {} ", ::strerror(errno));
                return -1;
            }
            already_send_bytes += send_bytes;
        }

        if(already_send_bytes == to_send_bytes){
            buf.clear();
        }else {
            buf = buf.substr(already_send_bytes);
        }
        return already_send_bytes;
    }
}