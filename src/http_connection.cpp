#include <cassert>
#include <spdlog/spdlog.h>
#include "http_connection.h"
#include "event_loop.h"
#include "channel.h"

namespace webserver{
    HttpConnection::HttpConnection(EventLoop* loop, TcpSocket conn_fd)
    : m_loop(loop), m_conn_fd(conn_fd), m_channel(std::make_shared<Channel>(conn_fd.get_fd(), loop)),
      m_handler(), m_state(Connected){
         assert(m_conn_fd.get_fd() > 0);
    }

    HttpConnection::~HttpConnection(){}
    
    void HttpConnection::set_default_cb(){
        m_channel->set_read_cb(std::bind(&HttpConnection::handle_read, this));
        m_channel->set_write_cb(std::bind(&HttpConnection::handle_write, this));
        m_channel->set_close_cb(std::bind(&HttpConnection::handle_close, this));
    }

    void HttpConnection::handle_read(){
        assert(m_loop->is_in_loop_thread());

        auto [nbytes, is_finished, buf] = m_conn_fd.recv();
        // error management
        if(nbytes < 0){
            m_state = DisConnected;
            handle_close();
            return;
        }else if(nbytes == 0 && m_state == Connected){
            m_state = DisConnected;
            handle_close();
            return;
        }else if(is_finished){
            m_state = DisConnecting;
            m_channel->unregister_read();
            m_conn_fd.shutdown(SHUT_RD);
            return;
        }
        m_state = Handle;
    }

    void HttpConnection::handle_write(){
        assert(m_loop->is_in_loop_thread());

        int nbytes = m_conn_fd.send(m_out_buffer);
        if(m_out_buffer.size() == 0){
            m_channel->unregister_write();
            if(m_state == DisConnecting){
                m_state = DisConnected;
                handle_close();
                return;
            }
        }
        if(nbytes < 0){
            m_state = DisConnected;
            handle_close();
        }
    }

    void HttpConnection::handle_close(){
        assert(m_loop->is_in_loop_thread());

        auto guard = m_handler.lock();
        if(guard == nullptr){
            spdlog::error("Handler destroyed before connection!");
        }

        m_channel -> unregister_all();
        m_loop -> remove_channel(m_channel);
    }

    void HttpConnection::send(const void* data, int len){
        assert(m_loop -> is_in_loop_thread());
        const char* ptr = static_cast<const char*>(data);

        int left = m_out_buffer.size();
        m_out_buffer.resize(left + len);
        std::copy(ptr, ptr + len, m_out_buffer.data() + left);

        m_channel->register_write();
    }

    void HttpConnection::send(const std::string& data){
        send(data.data(), data.size());
    }

    void HttpConnection::shut_down(int how){
        m_conn_fd.shutdown(how);
    }
}