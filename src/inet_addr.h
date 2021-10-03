#pragma once

#include <string>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
namespace webserver{

/* Wrapper of a sockaddr_in.
*/
class InetAddr{

public:
    InetAddr() = default;
    explicit InetAddr(uint16_t port){
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    InetAddr(std::string ip, uint16_t port){
        bzero(&m_addr, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr);
    }
    InetAddr(sockaddr_in addr) { m_addr = std::move(addr);}

    const sockaddr_in& get_addr() const { return m_addr;}
    void set_addr(sockaddr_in addr) { m_addr = std::move(addr); }

    std::string to_ip_string() const { 
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &m_addr.sin_addr, ip, sizeof(ip));
        return std::string(ip);
    }

    std::string to_ip_port_string() const{
        char ip[INET_ADDRSTRLEN];
        
        inet_ntop(AF_INET, &m_addr.sin_addr, ip, sizeof(ip));
        uint16_t port = ntohs(m_addr.sin_port);
        return std::string(ip) + ":" + std::to_string(port);
    }
private:
    sockaddr_in m_addr;

};

}