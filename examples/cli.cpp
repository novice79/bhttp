#include <bhttp/client.hpp>

// examples/app 's corresponding client
using namespace std;
int main(int argc, char **argv) 
{
    bhttp::get("http://127.0.0.1:8888",
    [](auto res, const SimpleWeb::error_code &ec) {
      if(!ec)
        cout << "Response content: " << res->content.rdbuf() << endl;
    });

    bhttp::put("https://127.0.0.1:9999/update", "some data",
    [](auto res, const SimpleWeb::error_code &ec) {
      if(!ec)
        cout << "Response content: " << res->content.rdbuf() << endl;
    });
    boost::json::value json{
        {"firstName", "David"},
        {"lastName", "Johnson"},
        {"age", 43}
    };
    bhttp::post("http://127.0.0.1:8888/json", 
        boost::json::serialize(json), 
        {
            {"Content-Type", "application/json"}
        },
        [](auto res, const SimpleWeb::error_code &ec) {
        if(!ec)
            cout << "Response content: " << res->content.string() << endl;
        }
    );
    bhttp::ws("wss://localhost:9999/hello", {
        .on_open = [](WsCnn* conn){
            printf("wss connect to server succeed\n");
            conn->send( "hello world" );
        },
        .on_close = [](WsCnn* conn, int status, const std::string& reason){
            printf("wss close from server\n");
        },
        .on_error = [](WsCnn* conn, const boost::system::error_code & ec){
            printf("wss error: %s\n", ec.message().c_str() );
        },
        .on_message = [](WsCnn* conn, std::string msg){
            printf("wss received new msg: %s from server\n", msg.c_str() );
            // cout << "Client: Sending close connection" << endl;
            // conn->send_close(1000);
        }
    });
    bhttp::ws("ws://localhost:8888/ws", {
            .on_open = [](WsCnn* conn){
                printf("ws client connect to server succeed\n");
                conn->send( "hello world from ws client" );
            },
            .on_message = [](WsCnn* conn, std::string msg){
                printf("ws client received new msg: %s from server\n", msg.c_str() );
                // if not active close from server, it will run forever untill server down
                // cout << "ws client: Sending close connection" << endl;
                // conn->send_close(1000);
            }
        },
        // detach from current thread or not. default: true (i.e. run in new thread)
        true
    );
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(20000s);
    return 0;
}