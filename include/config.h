#pragma once
#include <string>

namespace web{
    struct WebConfig{
    std::string listen_addr;
    short listen_port;
    std::string index_directory;

    WebConfig(std::string listen_addr, short listen_port, std::string index_directory) 
        : listen_addr(listen_addr), 
          listen_port(listen_port), 
          index_directory(index_directory) {}
        
    bool operator==(const WebConfig& rhs) const {
        return listen_addr == rhs.listen_addr &&
            listen_port == rhs.listen_port &&
            index_directory == rhs.index_directory;
    }
};

WebConfig LoadConfig(const std::string& config_file_path);
}