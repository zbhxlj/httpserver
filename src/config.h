#pragma once
#include <string>

namespace webserver{
    /* Configuration Instance. 
       In chanrge of load configuration.
    */
    struct WebConfig{
        std::string m_listen_addr;
        short m_listen_port;
        std::string m_index_directory;
        int m_thread_pool_num;
        int m_reserved_task_num;
        int m_poll_timeout;

        WebConfig(std::string listen_addr, short listen_port, std::string index_directory,
            int thread_pool_num, int reserved_task_num, int poll_timeout) 
            : m_listen_addr(listen_addr), 
                m_listen_port(listen_port), 
                m_index_directory(index_directory),
                m_thread_pool_num(thread_pool_num),
                m_reserved_task_num(reserved_task_num), 
                m_poll_timeout(poll_timeout){}
                
        bool operator==(const WebConfig& rhs) const {
            return m_listen_addr == rhs.m_listen_addr &&
                    m_listen_port == rhs.m_listen_port &&
                    m_index_directory == rhs.m_index_directory &&
                    m_thread_pool_num == rhs.m_thread_pool_num && 
                    m_reserved_task_num == rhs.m_reserved_task_num &&
                    m_poll_timeout == rhs.m_poll_timeout;
        }
};

    WebConfig load_config(const std::string& config_file_path);
}