#pragma once

#include "channel.h"
#include "noncopyable.h"
#include <list>
#include <unordered_map>
#include <unordered_set>
namespace webserver {

class HttpHandler;
class EventLoop;
class Channel;
class Timer;
/* Manage all Http Handler in a loop.
 */
class HttpManager : public Noncopyable {

  public:
    using channel_ptr = std::shared_ptr<Channel>;
    using http_handler_ptr = std::shared_ptr<HttpHandler>;
    /* Channel's next timeout timestamp.
     */
    using entry = std::pair<channel_ptr, timeval>;
    /* Position in timeout list.
     */
    using timer_node = std::list<entry>::iterator;

    HttpManager(EventLoop *loop);
    ~HttpManager();
    /* Handle http request.
     */
    void handler(channel_ptr &channel);
    void add_http_connection(http_handler_ptr &http_handler);
    void del_http_connection(channel_ptr &channel);
    /* Reset a channel's keep-alive timeout.
     */
    void flush_keep_alive(const channel_ptr &channel,
                          HttpManager::timer_node &node);
    /* Delete timed out keep-alive connection.
     */
    void handle_expire_event();

  private:
    EventLoop *m_loop;
    std::unique_ptr<Timer> m_timer;
    /* Connect channel with its HttpHandler.
       So when we get channel's corresponding HttpHandler.
     */
    std::unordered_map<channel_ptr, http_handler_ptr, ChannelHash> m_http_map;
    /* List of keep-alive connection, saving their timeout timestamp.
     */
    std::list<entry> m_keep_alive_list;
    /* Set of keep-alive connection.
     */
    std::unordered_set<channel_ptr, ChannelHash> m_keep_alive_set;
};

} // namespace webserver