#include <yaml-cpp/node/parse.h>
#include <string>

#include <yaml-cpp/yaml.h>
#include "config.h"

namespace webserver{
    WebConfig load_config(const std::string& config_file_path){
        YAML::Node config = YAML::LoadFile(config_file_path);
        return WebConfig(
            config["server"]["listen_addr"].as<std::string>(), 
            config["server"]["listen_port"].as<short>(),
            config["server"]["index_directory"].as<std::string>(),
            config["server"]["thread_pool_num"].as<int>(),
            config["server"]["reserved_task_num"].as<int>(),
            config["server"]["poll_timeout"].as<int>()
        );
    } 
}