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
        std::function<void(std::shared_ptr<typename T::Response>, const error_code &)> && cb)
    {
        auto self(this->shared_from_this());
        client_->request(method, path_, content_, cb);
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
    WsCli(std::string url)
    :client_(std::make_shared<T>(url))
    {
        // printf("url=%s\n", url.c_str() );
    }
    void req(WSCB &&cb)
    {
        auto self(this->shared_from_this());
        client_->on_open = [&cb](std::shared_ptr<typename T::Connection> cnn) 
        {
            if(cb.on_open)
            {
                WsCnnWrapper wsr(cnn);
                cb.on_open(&wsr);
            }
        };
        client_->on_close = [&cb](std::shared_ptr<typename T::Connection> cnn, int status, const string &reason) 
        {
            if(cb.on_close)
            {
                WsCnnWrapper wsr(cnn);
                cb.on_close(&wsr, status, reason);
            }
        };
        client_->on_error = [&cb](std::shared_ptr<typename T::Connection> cnn, const SimpleWeb::error_code &ec) 
        {
            if(cb.on_error)
            {
                WsCnnWrapper wsr(cnn);
                cb.on_error(&wsr, ec);
            }
        };
        client_->on_message = [&cb](std::shared_ptr<typename T::Connection> cnn, 
                                 std::shared_ptr<typename T::InMessage> in_message) 
        {
            if(cb.on_message)
            {
                WsCnnWrapper wsr(cnn);
                cb.on_message(&wsr, in_message->string());
            }
        };

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
        auto cb
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
                )->req(method, cb);
                break;
            }
            if("https" == proto)
            {
                std::make_shared<HttpCli<HttpsClient>>(
                    addr,
                    std::move(path),
                    std::move(content)
                )->req(method, cb);
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
        auto cb
    )
    {
        return fetch(method, url, content, {}, std::move(cb) );
    }
    
    static int ws(
        std::string url, 
        WSCB &&cb
    )
    {
        auto [ec, proto, addr, path] = parse_url(url);
        if(ec != 0) return ec;
        do
        {
            if("ws" == proto)
            {
                std::make_shared<WsCli<WsClient>>( addr+path )->req( std::move(cb) );
                break;
            }
            if("wss" == proto)
            {
                std::make_shared<WsCli<WssClient>>( addr+path )->req( std::move(cb) );
                break;
            }
            return -1;
        } while (false);
        return 0;
    }
}