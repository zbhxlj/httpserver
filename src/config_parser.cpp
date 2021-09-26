#include <yaml-cpp/node/parse.h>
#include <string>

#include <yaml-cpp/yaml.h>
#include "../include/config.h"

namespace web{
    WebConfig LoadConfig(const std::string& config_file_path){
        YAML::Node config = YAML::LoadFile(config_file_path);
        return WebConfig(
            config["server"]["listen_addr"].as<std::string>(), 
            config["server"]["listen_port"].as<short>(),
            config["server"]["index_directory"].as<std::string>()
        );
    } 
}