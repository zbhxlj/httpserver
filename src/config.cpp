#include <string>
#include <yaml-cpp/node/parse.h>

#include "config.h"
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

namespace webserver {
    WebConfig global_config;
WebConfig load_config(const std::string &config_file_path) {
    YAML::Node config = YAML::LoadFile(config_file_path);
    auto listen_addr = config["server"]["listen_addr"].as<std::string>();
    auto listen_port = config["server"]["listen_port"].as<short>();
    auto index_directory = config["server"]["index_directory"].as<std::string>();
    auto thread_pool_num = config["server"]["thread_pool_num"].as<int>();
    auto reserved_task_num = config["server"]["reserved_task_num"].as<int>();
    auto poll_timeout = config["server"]["poll_timeout"].as<int>();
    
    spdlog::info("Server address : {}", listen_addr);
    spdlog::info("Server port : {}", listen_port);
    spdlog::info("Index directory : {}", index_directory);
    
    global_config = WebConfig(listen_addr, listen_port, index_directory, 
                thread_pool_num, reserved_task_num, poll_timeout);
    return global_config;
}
} // namespace webserver