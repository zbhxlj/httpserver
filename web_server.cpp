#include "src/config.h"
#include "src/event_loop.h"
#include "src/http_server.h"
#include <filesystem>
#include <spdlog/spdlog.h>
int main() {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %T.%f] [thread %t] %@  %v");
    spdlog::info("Config file path : {}", std::filesystem::current_path().string() + "/config.yaml");
    auto config = webserver::load_config(
        std::filesystem::current_path().string() + "/config.yaml");
    webserver::EventLoop main_loop;

    webserver::HttpServer server(&main_loop, config.listen_addr,
                                 config.listen_port, 8);
    server.start();

    main_loop.loop();
}