#pragma once
#include <boost/filesystem.hpp>

#include "app.hpp"
#include "client.hpp"

namespace fs = boost::filesystem;

class LuaSetup
{
    LuaSetup()
    {
        lua_.open_libraries();
        setup();
    }
    void setup()
    {
        auto bhttp = lua_["bhttp"].get_or_create<sol::table>();
        bhttp.set_function("get", 
            [this](std::string url, sol::function cb, bool detached = true) { 
                bhttp::get(
                    std::move(url),
                    [this, cb=std::move(cb)](auto res, const SimpleWeb::error_code &ec) {
                        auto lua_ret_str = res->content.string();
                        auto lua_ec = lua_.create_table_with(
                                "failed", (bool)ec,
                                "msg", ec.message()
                            );
                        cb(lua_ret_str, lua_ec);
                    },
                    detached
                );
            }
        );
    }
private:
    sol::state lua_;
public:
    LuaSetup(LuaSetup &other) = delete;
    void operator=(const LuaSetup &) = delete;
    static LuaSetup* instance()
    {
        static LuaSetup the_one;
        return &the_one;
    }
    ~LuaSetup()
    {
        
    }
    void run_file(fs::path lua_file)
    {
        // lua.script("print('bark bark bark!')");
        lua_.script_file(lua_file.string());
    }
};
