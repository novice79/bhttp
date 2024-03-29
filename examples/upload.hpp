#pragma once
#include <bhttp/app.hpp>


inline void upload(auto* app, const char* argv0) 
{
    auto store = Util::exe_path(argv0) / "store";
    app->cors()
    .serve_dir("*", Util::exe_path(argv0) / "www")
    .serve_dir("/store", store)
    .upload("^/upload$", store, [store](auto* app, auto path){
        printf("upload %s completed\n", path.c_str() );
        app->ws_broadcast("^/store$", json::serialize( Util::file_info(store) ) );
    })
    .post("^/del$", [store](auto* app, auto res, auto req){
        auto path = req->content.string();
        fs::remove_all(path);
        app->ws_broadcast("^/store$", json::serialize( Util::file_info(store) ) );
        res->write(SimpleWeb::StatusCode::success_ok);
    })
    .ws("^/store$", {
        .on_open = [store=std::move(store)](auto* app, auto conn){
            string fi{json::serialize( Util::file_info(store) )};
            conn->send( fi );
        }
    });
}