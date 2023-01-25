#pragma once


#include "client_http.hpp"
#include "client_https.hpp"
#include "client_ws.hpp"
#include "client_wss.hpp"

#include "util.hpp"

using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

template <typename T>
class HttpCli : public std::enable_shared_from_this<HttpCli<T>>
{
    // std::variant<
    //     std::shared_ptr<HttpClient>,
    //     std::shared_ptr<HttpsClient>,
    //     std::shared_ptr<WsClient>,
    //     std::shared_ptr<WssClient>
    // > client_;
    std::shared_ptr<T> client_;
    std::string path_, content_;
public:
    // ~HttpCli(){printf("~HttpCli()\n");}
    HttpCli(std::string addr, std::string &&path, std::string &&content)
    :client_(std::make_shared<T>(addr)), path_(std::move(path)),
    content_(std::move(content))
    {
    }
    void req(std::string method, 
            std::function<void(std::shared_ptr<typename T::Response>, const error_code &)> cb,
            bool detached = true
         )
    {
        auto self(this->shared_from_this());
        client_->request(method, path_, content_, std::move(cb) );
        if(detached)
        {
            std::thread( std::bind(&HttpCli::run, self) ).detach();
        }
        else
        {
            run();
        }
    }
    void run()
    {
        client_->io_service->run();
    }
};

struct WsCnn
{
    virtual void send(std::string msg) = 0;
    virtual void send_close(int status, std::string reason = "") = 0;
};
struct WSCB
{
    std::function<void(WsCnn*)> on_open;
    std::function<void(WsCnn*, int, const std::string&)> on_close;
    std::function<void(WsCnn*, const boost::system::error_code &)> on_error;
    std::function<void(WsCnn*, std::string)> on_message;
};
template <typename T>
class WsCli : public std::enable_shared_from_this<WsCli<T>>
{
    std::shared_ptr<T> client_;
    WSCB cb_;
    struct WsCnnWrapper : WsCnn
    {     
        WsCnnWrapper(std::shared_ptr<typename T::Connection> cnn)
        :cnn_(cnn)
        {}
        virtual void send(std::string msg)
        {
            cnn_->send(msg);
        }
        virtual void send_close(int status, std::string reason = "")
        {
            cnn_->send_close(status, reason);
        }
    private:
        std::shared_ptr<typename T::Connection> cnn_;
    };
public:
    WsCli(std::string url, WSCB &&cb)
    :client_(std::make_shared<T>(url)), cb_(std::move(cb))
    {
        // printf("url=%s\n", url.c_str() );
    }
    // ~WsCli()
    // {
    //     printf("~WsCli()\n");
    // }
    void req(bool detached = true)
    {
        auto self(this->shared_from_this());
        client_->on_open = [this](std::shared_ptr<typename T::Connection> cnn) 
        {
            if(cb_.on_open)
            {
                WsCnnWrapper wsr(cnn);
                cb_.on_open(&wsr);
            }
        };
        client_->on_close = [this](std::shared_ptr<typename T::Connection> cnn, int status, const string &reason) 
        {
            if(cb_.on_close)
            {
                WsCnnWrapper wsr(cnn);
                cb_.on_close(&wsr, status, reason);
            }
        };
        client_->on_error = [this](std::shared_ptr<typename T::Connection> cnn, const SimpleWeb::error_code &ec) 
        {
            if(cb_.on_error)
            {
                WsCnnWrapper wsr(cnn);
                cb_.on_error(&wsr, ec);
            }
        };
        client_->on_message = [this](std::shared_ptr<typename T::Connection> cnn, 
                                 std::shared_ptr<typename T::InMessage> in_message) 
        {
            if(cb_.on_message)
            {
                WsCnnWrapper wsr(cnn);
                cb_.on_message(&wsr, in_message->string());
            }
        };
        if(detached)
        {
            std::thread( std::bind(&WsCli::run, self) ).detach();
        }
        else
        {
            run();
        }
    }
    void run()
    {
        client_->start();
    }
};

namespace bhttp
{
    inline std::tuple<int, std::string, std::string, std::string>
    parse_url(std::string url)
    {
        boost::trim(url);
        static std::regex s_url_re ("(http[s]?|ws[s]?)://([^/]+)(.*)");
        static std::regex s_with_port_re ("[^:]+:[0-9]{2,5}");
        std::smatch sm;
        bool valid = std::regex_match (url, sm, s_url_re);
        if(!valid) return {-1,"","",""};
        std::string proto = sm[1].str();
        std::string addr = sm[2].str();
        std::string path = sm[3].str();
        if( !std::regex_match(addr, sm, s_with_port_re) ) addr += ":80";
        if( path.empty() ) path = "/";
        return {0, proto, addr, path};
    }
    
    static int fetch(
        std::string method, 
        std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    )
    {
        auto [ec, proto, addr, path] = parse_url(url);
        if(ec != 0) return ec;
        do
        {
            if("http" == proto)
            {
                std::make_shared<HttpCli<HttpClient>>(
                    addr,
                    std::move(path),
                    std::move(content)
                )->req(method, std::move(cb), detached);
                break;
            }
            if("https" == proto)
            {
                std::make_shared<HttpCli<HttpsClient>>(
                    addr,
                    std::move(path),
                    std::move(content)
                )->req(method, std::move(cb), detached);
                break;
            }
            return -1;
        } while (false);

        return 0;
    }
    static int fetch(
        std::string method, 
        std::string url, 
        std::string content, 
        auto cb,
        bool detached = true
    )
    {
        return fetch(method, url, content, {}, std::move(cb), detached );
    }
    inline int get(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("GET", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int get(std::string url, auto cb, bool detached = true)
    {return get(std::move(url), "", {}, std::move(cb), detached);}

    inline int post(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("POST", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int post(std::string url, std::string content, auto cb, bool detached = true)
    {return post(std::move(url), std::move(content), {}, std::move(cb), detached);}

    inline int options(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("OPTIONS", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int options(std::string url, std::string content, auto cb, bool detached = true)
    {return options(std::move(url), std::move(content), {}, std::move(cb), detached);}

    inline int del(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("DELETE", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int del(std::string url, std::string content, auto cb, bool detached = true)
    {return del(std::move(url), std::move(content), {}, std::move(cb), detached);}

    inline int patch(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("PATCH", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int patch(std::string url, std::string content, auto cb, bool detached = true)
    {return patch(std::move(url), std::move(content), {}, std::move(cb), detached);}

    inline int put(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("PUT", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int put(std::string url, std::string content, auto cb, bool detached = true)
    {return put(std::move(url), std::move(content), {}, std::move(cb), detached);}

    inline int head(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("HEAD", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int head(std::string url, std::string content, auto cb, bool detached = true)
    {return head(std::move(url), std::move(content), {}, std::move(cb), detached);}

    inline int connect(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("CONNECT", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int connect(std::string url, std::string content, auto cb, bool detached = true)
    {return connect(std::move(url), std::move(content), {}, std::move(cb), detached);}

    inline int trace(std::string url, 
        std::string content, 
        SimpleWeb::CaseInsensitiveMultimap header,
        auto cb,
        bool detached = true
    ){return fetch("TRACE", std::move(url), std::move(content), std::move(header), std::move(cb), detached);}
    inline int trace(std::string url, std::string content, auto cb, bool detached = true)
    {return trace(std::move(url), std::move(content), {}, std::move(cb), detached);}

    static int ws(
        std::string url, 
        WSCB &&cb,
        bool detached = true
    )
    {
        auto [ec, proto, addr, path] = parse_url(url);
        if(ec != 0) return ec;
        do
        {
            if("ws" == proto)
            {
                std::make_shared<WsCli<WsClient>>( addr+path, std::move(cb) )->req( detached );
                break;
            }
            if("wss" == proto)
            {
                std::make_shared<WsCli<WssClient>>( addr+path, std::move(cb) )->req( detached );
                break;
            }
            return -1;
        } while (false);
        return 0;
    }
}