#pragma once
#include <string>

namespace webserver{
    struct WebConfig{
        std::string m_listen_addr;
        short m_listen_port;
        std::string m_index_directory;

        WebConfig(std::string listen_addr, short listen_port, std::string index_directory) 
            : m_listen_addr(listen_addr), 
            m_listen_port(listen_port), 
            m_index_directory(index_directory) {}
            
        bool operator==(const WebConfig& rhs) const {
            return m_listen_addr == rhs.m_listen_addr &&
                m_listen_port == rhs.m_listen_port &&
                m_index_directory == rhs.m_index_directory;
        }
};

    WebConfig load_config(const std::string& config_file_path);
}