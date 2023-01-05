#include <bhttp/app.hpp>
// for json-example
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
using namespace boost::property_tree;
#include <sstream>
#include <boost/format.hpp>
using namespace std;
using boost::format;
int main(int argc, char **argv) 
{
// start a https/wss server
    // if expect it to run_detached(not block main thread), need to keep it in memory
    BHS app("misc/server.crt", "misc/server.key");
    // curl https://127.0.0.1:9999/info
    app.get("^/info$", [](auto res, auto req){
        stringstream s;
        s << "C++ boost https server, handled by thread: ";
        s << std::this_thread::get_id();
        cout<< s.str() << endl;
        res->write( s.str() );
    })
    // url: "wss://localhost:9999/hello"
    .ws("^/hello$", {
        .on_open = [](auto conn){
            printf("wss received new connection\n");
        },
        .on_close = [](auto conn, auto status, auto reason){
            printf("wss connection closed\n");
        },
        .on_error = [](auto conn, auto ec){
            printf("wss error: %s\n", ec.message().c_str() );
        },
        .on_message = [](auto conn, auto msg){
            printf("wss received new msg: %s\n", msg->string().c_str() );
            // echo back msg
            conn->send( str(format("%1% [threadID: %2%]") % msg->string() % std::this_thread::get_id() ) );
        }
    })
    // macos show number of CPU Cores: sysctl -n hw.ncpu
    // run async loop in cpu core number thread pool, default size: 1
    .threads( std::thread::hardware_concurrency() )
    .listen(9999).run_detached();

// and start another http/ws server
    BH()
    // so this can be used to serve spa[vuejs/reactjs/...] ui
    // curl http://localhost:8888/  -- serve files in www dir within executable path
    .serve_dir("*", Util::exe_path(argv[0]) / "www")
    // curl http://localhost:8888/store/ -- serve files in store dir within executable path
    .serve_dir("/store", Util::exe_path(argv[0]) / "store")
    // curl http://127.0.0.1:8888/info
    .get("^/info$", [](auto res, auto req){
        res->write("C++ boost http server");
    })
    // curl http://127.0.0.1:8888/match/123444
    .get("^/match/([0-9]+)$", [](auto res, auto req){
        res->write(req->path_match[1].str());
    })
    // curl -X POST http://127.0.0.1:8888/json \
    // -H 'Content-Type: application/json' \
    // -d '{"firstName":"John","lastName":"Smith","age":"43"}'
    .post("^/json$", [](auto res, auto req){
        try {
            ptree pt;
            read_json(req->content, pt);
            auto name = pt.get<string>("firstName") + " " + pt.get<string>("lastName");
            *res << "HTTP/1.1 200 OK\r\n"
                        << "Content-Length: " << name.length() << "\r\n\r\n"
                        << name;
        }
            catch(const exception &e) {
            *res << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                        << e.what();
        }
    })
    // url: "ws://localhost:8888/ws"
    .ws("^/ws$", {
        .on_open = [](auto conn){
            printf("received new websocket connection\n");
        },
        .on_close = [](auto conn, auto status, auto reason){
            printf("websocket connection closed\n");
        },
        .on_error = [](auto conn, auto ec){
            printf("websocket error: %s\n", ec.message().c_str() );
        },
        .on_message = [](auto conn, auto msg){
            printf("received new msg: %s\n", msg->string().c_str() );
            // echo back msg
            conn->send( str(format("%1% [threadID: %2%]") % msg->string() % std::this_thread::get_id() ) );
        }
    })
    .cron_job([](auto* app){
      printf("test cron job every two seconds 3 times\n");  
    }, 2, 3)
    .cron_job([](auto* app){
    //   printf("run cron job every four seconds forever\n");  
        // parameters: 1. ws endpoint[regex string], 2. msg to be sent
        app->ws_broadcast("^/ws$", "this msg to every client every four seconds");
    }, 4, 0)
    .listen(8888).run();
    return 0;
}