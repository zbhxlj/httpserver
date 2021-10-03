#include <cassert>
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "http_handler.h"
#include "http_connection.h"
#include "event_loop.h"
namespace webserver{ 
    const char* HttpHandler::Method[] = {"GET"};
    const char* HttpHandler::Version[] = {"HTTP/1.0", "HTTP/1.1"};

    void HttpHandler::set_method(const std::string& method){
        // now only support GET
        if(method == Method[0]){
            m_method = static_cast<HttpMethod>(0);
        }
        else {
            spdlog::error("Method {} not suported", method);
            abort();
        }
    }

    void HttpHandler::set_url(const std::string& url){
        m_url = url;
    }

    void HttpHandler::set_version(const std::string& version){
        // only support HTTP/1.0 HTTP/1.1 
        bool matched = false;
        for(int i = 0; i < 2; i++){
            if(version == Version[i]){
                m_version = static_cast<HttpVersion>(i);
                matched = true;
            }
        }
        if(!matched){
            spdlog::error("Not supported version {}", version);
            abort();
        }
    }

    void HttpHandler::set_header(const std::string& key, const std::string& value){ 
        m_headers[key] = value;
    }

    HttpHandler::HttpHandler(EventLoop* loop, socket_ptr conn_fd)
    :m_loop(loop), m_conn_fd(conn_fd), m_connection(std::make_unique<HttpConnection>(loop, conn_fd)), 
     m_state(Start), m_is_keep_alive(false){
         assert(m_conn_fd->get_fd() > 0);
    }

    HttpHandler::~HttpHandler() = default;

    void HttpHandler::new_connection(){
        auto channel = m_connection->get_channel();

        m_connection->set_default_cb();
        m_connection->set_handler(shared_from_this());
        channel->register_read();
    }

    void HttpHandler::handle_http_request(){
        /*  Current position.
         */
        int bpos = 0;
        
        std::string buffer = m_connection->get_recv_buffer();
        spdlog::debug("buf = {}", buffer);
        HttpConnection::ConnState connState = m_connection->get_state();
        spdlog::debug("state = {}", connState);
        if(connState == HttpConnection::Error)
        {
            m_state = Start;	
            goto __err;
        }
        
        /* No data available. 
         */
        if(buffer.empty()) 
        {
            m_state = Start;
            goto __err;
        }
        
        bpos = parse_request_line(buffer, bpos);
        m_state = ParseRequestLine;
        
        bpos = parse_header(buffer, bpos);
        m_state = ParseHeader;

        m_state = ParseDone;
        
    __err:
        /*  Send http response.
         */
        respond_request();
        /* Clean up connection depends on keep-alive flag.
         */
        handle_keep_alive();
    }

    int HttpHandler::parse_request_line(std::string& buf, int bpos){
        std::string::size_type epos = buf.find("\r\n", bpos);
        
        /* Parse method
         */
        std::string::size_type space = buf.find(" ", bpos);
        set_method(buf.substr(bpos, space-bpos));
        spdlog::debug("Http method : {}", m_method);
        
        /* Parse url
         */
        bpos = space+1;
        space = buf.find(" ", bpos);
        set_url(buf.substr(bpos, space-bpos));
        spdlog::debug("Http url : {}", m_url);
        
        /* Parse version
         */
        bpos = space+1;
        set_version(buf.substr(bpos, epos-bpos));
        spdlog::debug("Http version : {}", m_version);
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
            spdlog::debug("{}: {}", key, value);
            bpos = epos+2;
        }
        
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
	
    spdlog::debug("http response header : {}", header);
    spdlog::debug("http response body : {}", body);
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
        spdlog::debug("http response header : {}", header);
        spdlog::debug("http response body : {}", body);
    }
    
    void HttpHandler::respond_request(){
        std::string filename("source/");
        std::string context;
        
        /* Parse error.
         */
        // if(m_state != ParseDone)
        // {
        //     //bad request 400
        //     spdlog::info("Parse state = {}", m_state);
        //     bad_request(400, "bad request");
        //     return ;
        // }
        
        if(m_url == "/")
        {
            filename += "index.html";
        }
        else 
        {
            /* Delete leading "/".
             */
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
        
        /* Fetch resources.
         */
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
            /* Close non keep-alive connection.
             */
            m_connection->set_state(HttpConnection::DisConnecting);
            m_connection->get_channel()->unregister_read();
            m_connection->shut_down(SHUT_RD);	
            
            return ;
        }
        
        /* If peer closed the write end, then we close.
         */
        HttpConnection::ConnState connState = m_connection->get_state();
        if(connState == HttpConnection::DisConnecting) return ;
        
        // Reset state.
        reset();
    }

    void HttpHandler::reset(){
        m_loop->flush_keep_alive(m_connection->get_channel(), m_timer_node);
        
        m_headers.clear();
        m_url.clear();
        m_state= Start;
        
        m_connection->set_state(HttpConnection::Handle);
    }
}