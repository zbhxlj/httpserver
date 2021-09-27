#pragma once
#include <string>

namespace webserver{
    struct WebConfig{
        std::string m_listen_addr;
        short m_listen_port;
        std::string m_index_directory;
        int m_thread_pool_num;

        WebConfig(std::string listen_addr, short listen_port, std::string index_directory,
            int thread_pool_num) 
            : m_listen_addr(listen_addr), 
            m_listen_port(listen_port), 
            m_index_directory(index_directory),
            m_thread_pool_num(thread_pool_num){}
            
        bool operator==(const WebConfig& rhs) const {
            return m_listen_addr == rhs.m_listen_addr &&
                m_listen_port == rhs.m_listen_port &&
                m_index_directory == rhs.m_index_directory &&
                m_thread_pool_num == rhs.m_thread_pool_num;
        }
};

    WebConfig load_config(const std::string& config_file_path);
}