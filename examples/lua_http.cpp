
#include <bhttp/app.hpp>

int main(int argc, char **argv) 
{
    Util::test_lua(Util::exe_path(argv[0]) / "test.lua");
    return 0;
}