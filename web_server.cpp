#include "src/config.h"
#include "src/event_loop.h"
#include "src/http_server.h"
#include <filesystem>
#include <spdlog/spdlog.h>
int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %T.%f] [thread %t] %@  %v");
    auto config = webserver::load_config(
        std::filesystem::current_path().string() + "/config.yaml");
    webserver::EventLoop main_loop;

    webserver::HttpServer server(&main_loop, config.listen_addr,
                                 config.listen_port, 3);
    server.start();

    main_loop.loop();
}