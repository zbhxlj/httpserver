#pragma once

#include "inet_addr.h"
#include "noncopyable.h"
#include <memory>

#define MAX_BUF_SIZE 4096
namespace webserver {
/* TcpSocket is a wrapper of a socket fd.
   In charge of its open、read、write、close.
*/
class TcpSocket : public Noncopyable {

  public:
    using socket_ptr = std::shared_ptr<TcpSocket>;
    TcpSocket() = default;
    /* Construct listen fd.
       Default set Non_Block and Reuse_addr.
    */
    explicit TcpSocket(std::string ip, uint16_t port);
    /* Construct accept fd.
       Default set Non_Block.
    */
    explicit TcpSocket(int sock_fd, InetAddr addr);
    /* Close socket fd here.
     */
    ~TcpSocket();
    void close();
    /* Bool indicates whether a successful accept.
     */
    std::pair<bool, socket_ptr> accept();
    /* Shutdown gracefully.
       i.e. half shutdown
    */
    void shutdown(int type);
    void ignore_sigpipe();
    /* Recv on socket fd with Non_Block set.
       Need to check the return value(bytes recv).
       -1 indicates unexpected error(not EAGAIN or EWOULDBLOCK or EINTR).
    */
    std::pair<int, bool> recv(std::string &buf);
    /* Send on socket fd with Non_Block set.
       Need to check the return value(bytes send).
       -1 indicates unexpected error(not EAGAIN or EWOULDBLOCK or EINTR).
    */
    int send(std::string &buf);
    int get_fd() const { return m_socket_fd; }

  private:
    int m_socket_fd;
    InetAddr m_addr;
};

} // namespace webserver