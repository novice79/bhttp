
#include <bhttp/lua_setup.hpp>
int main(int argc, char **argv) 
{
    LuaSetup::instance()->run_file(Util::exe_path(argv[0]) / "test.lua");

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(20000s);
    return 0;
}