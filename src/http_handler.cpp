#include <cassert>
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "http_handler.h"
#include "http_connection.h"
namespace webserver{ 
    const char* HttpHandler::Method[] = {"GET"};
    const char* HttpHandler::Version[] = {"HTTP/1.0", "HTTP/1.1"};

    HttpHandler::HttpHandler(EventLoop* loop, TcpSocket conn_fd)
    :m_loop(loop), m_conn_fd(conn_fd), m_connection(std::make_unique<HttpConnection>(loop, conn_fd)), 
     m_state(Start), m_is_keep_alive(false){
         assert(m_conn_fd.get_fd() > 0);
    }

    HttpHandler::~HttpHandler() = default;

    void HttpHandler::new_connection(){
        auto channel = m_connection->get_channel();

        m_connection->set_default_cb();
        m_connection->set_handler(shared_from_this());
        channel->register_read();
    }

    void HttpHandler::handle_http_request(){
        /* bpos当前位置，epos下一行位置 */
        int bpos = 0, epos = 0;
        
        /* ROV优化, 不必忧心效率 */
        std::string buffer = m_connection->get_recv_buffer();
        HttpConnection::ConnState connState = m_connection->get_state();
        if(connState == HttpConnection::Error)
        {
            m_state = Start;	/* 跳过解析环节，回复404 bad request */
            goto __err;
        }
        
        /* 请求到来，但无数据 */
        if(buffer.empty()) 
        {
            m_state = Start;
            goto __err;
        }
        
        bpos = parse_request_line(buffer, bpos);
        bpos = parse_header(buffer, bpos);
        m_state = ParseDone;
        
    __err:
        /* 根据解析状态，返回结果 */
        respond_request();
        
        /* 连接处理：断开 or 保持 */
        handle_keep_alive();
    }

    int HttpHandler::parse_request_line(std::string& buf, int bpos){
        std::string::size_type epos = buf.find("\r\n", bpos);
        
        /* 解析请求方法 */
        std::string::size_type space = buf.find(" ", bpos);
        set_method(buf.substr(bpos, space-bpos));
        
        /* 解析请求资源路径 */
        bpos = space+1;
        space = buf.find(" ", bpos);
        set_url(buf.substr(bpos, space-bpos));
        
        /* 解析Http版本号 */
        bpos = space+1;
        set_version(buf.substr(bpos, epos-bpos));
        if(m_version == HttpV11) m_is_keep_alive = true;        
        return epos+2;
    }

    int HttpHandler::parse_header(std::string& buf, int bpos){
        std::string::size_type epos = bpos;
	
        while(static_cast<int>(epos = buf.find("\r\n", bpos)) != bpos)
        {
            std::string::size_type sep = buf.find(":", bpos);
            
            while(buf[bpos] == ' ') bpos++;
            std::string key(buf.substr(bpos,sep-bpos));
            
            sep += 1;
            while(buf[sep] == ' ') sep++;
            std::string value(buf.substr(sep, epos-sep));
            
            set_header(key, value);
            
            bpos = epos+2;
        }
        
        /* Keepalive判断 */
        if(m_headers.find("Connection") != m_headers.end())
        {
            if(m_headers["Connection"] == "keep-alive" || 
            m_headers["Connection"] == "Keep-Alive") 
            {
                m_is_keep_alive = true;
            }
            else if(m_headers["Connection"] == "close" ||
                    m_headers["Connection"] == "Close")
            {
                m_is_keep_alive = false;
            }
        }
        return epos + 2;
    }

    void HttpHandler::bad_request(int num, const std::string &note)
{
	std::string body;
	std::string header;
	
	header += "HTTP/1.1 " + std::to_string(num) + 
	          " " + note + "\r\n";
	header += "Content-Type: text/html\r\n";
	
	if(! m_is_keep_alive) header += "Connection: close\r\n";
	else             header += "Connection: Keep-Alive\r\n";
	
	body += "<html><title>呀~出错了</title>";
	body += "<body>" + std::to_string(num) + " " + note;
	body += "</body></html>";
	
	header += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	header += "Server: ZBH's HttpServer\r\n\r\n";
	
	m_connection->send(header);
	m_connection->send(body);
}

    void HttpHandler::on_request(const std::string& body){
        std::string header;
	
        header += "HTTP/1.1 200 OK\r\n";
        header += "Content-Type: text/html\r\n";
        
        if(! m_is_keep_alive) header += "Connection: close\r\n";
        else             header += "Connection: Keep-Alive\r\n";
        
        header += "Content-Length: " + 
                std::to_string(body.size()) + "\r\n";
        header += "Server: ZBH's HttpServer\r\n\r\n";
        
        m_connection->send(header);
        m_connection->send(body);
    }
    
    void HttpHandler::respond_request(){
        std::string filename("source/");
        std::string context;
        
        /* 根据解析状态，响应Http请求 */
        if(m_state != ParseDone)
        {
            //bad request 400
            bad_request(400, "bad request");
            return ;
        }
        
        /* 解析请求资源 */
        if(m_url == "/")
        {
            //默认返回index.html页面
            filename += "index.html";
        }
        else 
        {
            /* 不能原地赋值!! */
            std::string path = m_url.substr(1);
            
            //for webbench test!
            // TODO
            // if(path == "hello")
            // {
            //     std::string hello("Hello, I'm WebServer.");
            //     onRequest(hello);
            //     return ;
            // }
            if(::access((filename+path).c_str(), F_OK) < 0)
            {
                spdlog::warn("File {} not found", filename + path);
                bad_request(404, "Not Found");
                return ;
            }
            filename += path;
        }
        
        /* 返回页面 */
        struct stat st;
        if(::stat(filename.c_str(), &st) < 0)
        {
            spdlog::warn("Access file {} permission denied", filename);
            bad_request(404, "Access Permission Denied");
            return ;
        }
        
        int fd = ::open(filename.c_str(), O_RDONLY);
        if(fd < 0)
        {
            spdlog::warn("Cannot open file {}", filename);
            bad_request(404, "Open failed");
            return ;
        }
        
        void *map_file = ::mmap(NULL, st.st_size, PROT_READ, 
                            MAP_PRIVATE, fd, 0);
        if(map_file == MAP_FAILED)
        {
            spdlog::warn("mmap file {} failed", filename);
            bad_request(404, "Mmap failed");
            return ;
        }
        
        char *pf = static_cast<char *>(map_file);
        context = std::string(pf, pf + st.st_size);
        on_request(context);
        
        ::close(fd);
        ::munmap(map_file, st.st_size);
    }

    void HttpHandler::handle_keep_alive (){
        if(! m_is_keep_alive){
            /* 关闭非keepalive连接，并返回 */
            m_connection->set_state(HttpConnection::DisConnecting);
            m_connection->get_channel()->unregister_read();
            m_connection->shut_down(SHUT_RD);	/* 关闭读半部 */
            
            return ;
        }
        
        /* 对方关闭了， 我们也关闭返回 */
        HttpConnection::ConnState connState = m_connection->get_state();
        if(connState == HttpConnection::DisConnecting) return ;
        
        // 重置状态
        reset();
    }

    void HttpHandler::reset(){
        /* 刷新keepalive时间 */
        m_loop->flush_keep_alive(m_connection->get_channel(), m_timer_node);
        
        /* 清理工作，为下次接受请求做准备 */
        m_headers.clear();
        m_url.clear();
        m_state= Start;
        
        /* 重置Httpconnection状态 */
        m_connection->set_state(HttpConnection::Handle);
    }
}