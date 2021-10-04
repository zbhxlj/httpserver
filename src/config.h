#pragma once
#include <string>

namespace webserver {
/* Configuration Instance.
   In chanrge of load configuration.
*/
struct WebConfig {
    std::string listen_addr;
    short listen_port;
    std::string index_directory;
    int thread_pool_num;
    int reserved_task_num;
    int poll_timeout;

    WebConfig(std::string listen_addr, short listen_port,
              std::string index_directory, int thread_pool_num,
              int reserved_task_num, int poll_timeout)
        : listen_addr(listen_addr), listen_port(listen_port),
          index_directory(index_directory), thread_pool_num(thread_pool_num),
          reserved_task_num(reserved_task_num), poll_timeout(poll_timeout) {}

    bool operator==(const WebConfig &rhs) const {
        return listen_addr == rhs.listen_addr &&
               listen_port == rhs.listen_port &&
               index_directory == rhs.index_directory &&
               thread_pool_num == rhs.thread_pool_num &&
               reserved_task_num == rhs.reserved_task_num &&
               poll_timeout == rhs.poll_timeout;
    }
};

WebConfig load_config(const std::string &config_file_path);
} // namespace webserver