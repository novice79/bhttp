#pragma once

#include "server_ws.hpp"
#include "server_http.hpp"
#include "server_https.hpp"
#include "server_wss.hpp"
#include "upload_handler.hpp"
#include "util.hpp"
namespace ph = std::placeholders;
namespace bpt = boost::posix_time;
class Timer : public std::enable_shared_from_this<Timer>
{
    boost::asio::deadline_timer routine_timer_;
    int repeat_;
    // bpt::hours(long), bpt::minutes(long), bpt::seconds(long), 
    // bpt::milliseconds(long), bpt::microseconds(long), bpt::nanoseconds(long)
    bpt::time_duration dur_;
    std::function<void()> cb_;
public:
    // ~Timer(){printf("~Timer()\n");}
    Timer(
        boost::asio::io_context &io, 
        std::function<void()> cb, 
        bpt::time_duration dur = bpt::seconds(1), 
        int repeat = 1
    )
    :routine_timer_(io, dur),
    dur_(dur), repeat_(repeat), cb_(cb)
    {

    }
    void wait()
    {
        auto self(shared_from_this());
        routine_timer_.async_wait( std::bind(&Timer::routine, self, ph::_1, &routine_timer_) );
    }
    void routine(const boost::system::error_code& /*e*/, boost::asio::deadline_timer* t)
    {
        // do something here
        cb_();
        if(repeat_ > 0 && --repeat_ == 0) return;
        t->expires_at(t->expires_at() + dur_);
        t->async_wait(std::bind(&Timer::routine, shared_from_this(), ph::_1, t));
    }
};

template <bool SSL>
class App 
{
    using HttpServer = std::conditional_t<
        SSL,
        SimpleWeb::Server<SimpleWeb::HTTPS>,
        SimpleWeb::Server<SimpleWeb::HTTP> >;
    typedef std::conditional_t<
        SSL, 
        SimpleWeb::SocketServer<SimpleWeb::WSS>,
        SimpleWeb::SocketServer<SimpleWeb::WS> > WsServer;

    WsServer ws_;
    HttpServer server_;
public:
    struct WSCB
    {
        std::function<void(App*, std::shared_ptr<typename WsServer::Connection>) > on_open;
        std::function<void(App*, std::shared_ptr<typename WsServer::Connection>, int, const std::string&) > on_close;
        std::function<void(App*, std::shared_ptr<typename WsServer::Connection>, const boost::system::error_code &) > on_error;
        std::function<void(App*, std::shared_ptr<typename WsServer::Connection>, std::shared_ptr<typename WsServer::InMessage>) > on_message;
    };
    App ( App && ) = default;
    App &  operator= ( App && ) = default;
    App ( const App & ) = delete;
    App & operator= ( const App & ) = delete;
    App()
    {emplace_ws();}
    App(std::string crt, std::string key)
    :server_(crt, key),ws_(crt, key)
    {emplace_ws();}
    ~App()
    {
        printf("~App()\n");
        server_.stop();
    }
    App&& use(std::function<void(App*)> doer)
    {   
        doer(this);
        return std::move(*this);
    }
    App&& upload(std::string vp, fs::path dir, std::function<void(App*, std::string)> cb = nullptr)
    {
        this->post(vp, [dir=std::move(dir), cb=std::move(cb), this](auto* app, auto res, auto req)
        {
            static UploadHandler uh( std::move(dir), std::bind(cb, this, ph::_1) );
            static SimpleWeb::CaseInsensitiveMultimap header
            {
                {"Content-Type", "text/plain; charset=utf-8"},
                {"Access-Control-Allow-Origin", "*"}
            };
            int ret = uh.write( req->content.string() );
            if( 0 == ret)
            {
                res->write(SimpleWeb::StatusCode::success_ok, header);
            }
            else
            {
                res->write(SimpleWeb::StatusCode::client_error_bad_request, header);
            }
        });    
        return std::move(*this);
    }
    App&& serve_dir(std::string vp, fs::path dir)
    {
        return serve_dir(vp, dir.string() );
    }
    App&& serve_dir(std::string vp, std::string dir)
    {
        class FileSvr : public std::enable_shared_from_this<FileSvr>
        {
            // Read and send 128 KB at a time
            enum{ buf_size = 131072}; 
            std::array<char, buf_size> buf_;
            std::shared_ptr<typename HttpServer::Response> res_;
            uint64_t len_;
            std::shared_ptr<std::ifstream> ifs_;
            void send_partial()
            {
                auto self(this->shared_from_this());
                if(len_ <= 0) return;
                size_t read_len = std::min(len_, (uint64_t)buf_size);
                std::streamsize read_length = ifs_->read( &buf_[0], static_cast<std::streamsize>(read_len) ).gcount();
                if (read_length > 0)
                {
                    res_->write(&buf_[0], read_length);
                    // std::cout << "write buff len = " << read_length << std::endl;
                    res_->send([self,read_length](const SimpleWeb::error_code &ec) {
                        if (!ec)
                        {
                            self->len_ -= read_length;
                            self->send_partial();
                        }                           
                        else
                        {
                            // std::cerr << "Connection interrupted: " << ec.message() << std::endl;
                        }                          
                    });  
                }
            }
        public:
            FileSvr(std::shared_ptr<typename HttpServer::Response> res, std::shared_ptr<std::ifstream> ifs, uint64_t len)
            :res_(res),ifs_(ifs),len_(len)
            {}
            ~FileSvr()
            {
                // std::cout << "~FileSvr()" << std::endl;
            }
            static void serve(
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req, 
                fs::path path,
                fs::path www_root)
            {
                using namespace std;
                try
                {
                    if (fs::is_directory(path))
                    {
                        path /= "index.html";
                    }
                    else 
                    if( !fs::exists(path) )
                    {
                        // This is for spa routing
                        path = www_root / "index.html";
                    }    
                    if( !fs::exists(path) )
                    {
                        throw invalid_argument(path.string() + " does not exist");
                    } 
                    SimpleWeb::CaseInsensitiveMultimap header;
                    // enable Cache-Control, except pac file
                    if( Util::is_pac(path.string()) )
                    {
                        header.emplace("Cache-Control", "no-cache, no-store, must-revalidate");
                    }
                    else
                    {
                        header.emplace("Cache-Control", "max-age=86400");
                    }
                    auto ifs = std::make_shared<ifstream>();
                    ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

                    if (*ifs)
                    {
                        header.emplace("Content-Type", Util::mime_type(path.string()) );
                        // for firefox need to know you can handle range seeking
                        header.emplace("Accept-Ranges", "bytes");
                        uint64_t file_len = ifs->tellg();
                        ifs->seekg(0, ios::beg);
                        uint64_t length = file_len;
                        auto it = req->header.find("Range");
                        if(it != req->header.end()) {
                            auto range = it->second;
                            boost::replace_all(range, "bytes=", "");
                            auto vs = Util::split(range, "-");
                            uint64_t begin = stoull( vs[0] );
                            uint64_t end = vs[1] == "" ? file_len : stoull( vs[1] );
                            // [begin, end] not [begin, end)
                            end = std::min( file_len - 1, end );
                            length = (end - begin) + 1;
                            range = "bytes " + vs[0] + "-" + to_string(end) + "/" + to_string(file_len);
                            header.emplace("Content-Range", range); 
                            header.emplace("Content-Length", to_string(length));
                            res->write(SimpleWeb::StatusCode::success_partial_content, header);
                            ifs->seekg(begin, ios::beg);
                        } 
                        else 
                        {
                            header.emplace("Content-Length", to_string(length));
                            res->write(header);
                        }
                        if(length > buf_size)
                        {
                            std::make_shared<FileSvr>(res, ifs, length)->send_partial();
                        }
                        else
                        {
                            std::vector<char> buf(length);
                            streamsize read_len = ifs->read( &buf[0], static_cast<streamsize>(length) ).gcount();
                            if(read_len > 0)
                            {
                                res->write(&buf[0], read_len);
                            }
                        }
                    }
                    else
                        throw invalid_argument("could not read file: " + path.string());
                }
                catch (const exception &e)
                {
                    res->write(
                        SimpleWeb::StatusCode::client_error_bad_request, 
                        "Could not open path " + req->path_match[1].str() + ": " + e.what());
                }
            }
        };
        boost::trim(vp);
        if(vp == "*" || vp.empty())
        {
            server_.default_resource["GET"] = [dir = move(dir)](auto res, auto req) mutable
            {
                auto web_root_path = fs::canonical(dir);
                auto path = web_root_path / req->path;
                FileSvr::serve(res, req, std::move(path), std::move(web_root_path));
            };
        }
        else
        {
            if(vp[0] != '^') vp = "^" + vp;
            if(vp[vp.length()-1] != '/') vp = vp + "/";
            vp += "(.*)$";
            // printf("vp=%s; dir=%s\n", vp.c_str(), dir.c_str());
            server_.resource[vp]["GET"] = [dir = move(dir)](auto res, auto req) mutable
            {
                auto fn = req->path_match[1].str();
                fn = Util::urlDecode(fn);
                auto web_root_path = fs::canonical(dir);
                auto path = web_root_path / fn;
                FileSvr::serve(res, req, std::move(path), std::move(web_root_path));
            };
        }
        return std::move(*this);
    }
    App&& cors()
    {
        server_.cors = true;
        // Deals with CORS requests
        server_.default_resource["OPTIONS"] = [](
            std::shared_ptr<typename HttpServer::Response> res, 
            std::shared_ptr<typename HttpServer::Request> req) 
        {
            try {
                // Set header fields
                SimpleWeb::CaseInsensitiveMultimap header;
                header.emplace("Content-Type", "text/plain; charset=utf-8");
                header.emplace("Access-Control-Allow-Origin", "*");
                header.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, DELETE");
                header.emplace("Access-Control-Max-Age", "1728000");
                header.emplace("Access-Control-Allow-Headers", "Authorization, Origin, X-Requested-With, Content-Type, Accept");

                res->write(SimpleWeb::StatusCode::success_ok, "", header);
            }
            catch(const exception &e) {
                res->write(SimpleWeb::StatusCode::client_error_bad_request, e.what());
            }
        };
        return std::move(*this);
    }
    App&& get(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["GET"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& post(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["POST"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& options(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["OPTIONS"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& del(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["DELETE"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& patch(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["PATCH"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& put(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["PUT"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& head(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["HEAD"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& connect(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["CONNECT"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& trace(
        std::string re,
        std::function<
            void(
                App*,
                std::shared_ptr<typename HttpServer::Response> res, 
                std::shared_ptr<typename HttpServer::Request> req
            )
        > cb)
    {
        server_.resource[re]["TRACE"] = std::bind(cb, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    App&& ws( std::string re, WSCB cb)
    {
        auto &ep = ws_.endpoint[re];
        if(cb.on_open) ep.on_open = std::bind(cb.on_open, this, ph::_1);
        if(cb.on_close) ep.on_close = std::bind(cb.on_close, this, ph::_1, ph::_2, ph::_3);
        if(cb.on_error) ep.on_error = std::bind(cb.on_error, this, ph::_1, ph::_2);
        if(cb.on_message) ep.on_message = std::bind(cb.on_message, this, ph::_1, ph::_2);
        return std::move(*this);
    }
    void ws_broadcast(std::string ep, std::string msg)
    {
        auto &cpp_channel_endpoint = ws_.endpoint[ep];
        for (auto &a_connection : cpp_channel_endpoint.get_connections())
            a_connection->send(msg);
    }
    App&& cron_job(std::function<void(App*)> cb, bpt::time_duration d = bpt::seconds(1), int repeat = 1)
    {
        std::make_shared<Timer>(
				*server_.inner_io(),
				std::bind(cb, this),
				d,
                repeat
			)->wait();
        return std::move(*this);
    }
    
    App&& threads(unsigned int num)
    {
        server_.config.thread_pool_size = num;
        return std::move(*this);
    }
    App&& listen(int port)
    {
        server_.config.port = port;
        return std::move(*this);
    }
    void run()
    {
        server_.start();
    }

    void run_detached()
    {
        std::thread t([this]()mutable {
            server_.start();
        });
        t.detach();
    }
private:
    void emplace_ws()
    {
        server_.on_upgrade = [this](auto &socket, auto request) 
        {
            auto connection = std::make_shared<typename WsServer::Connection>(std::move(socket));
            connection->method = std::move(request->method);
            connection->path = std::move(request->path);
            connection->query_string=std::move(request->query_string);
            connection->http_version = std::move(request->http_version);
            connection->header = std::move(request->header);
            ws_.upgrade(connection);
        };
    }

};
typedef App<false> BH;
typedef App<true> BHS;