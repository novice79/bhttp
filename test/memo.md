npm create vite@latest spa -- --template react

cd ~/cpp_libs/lua-5.4.4
make macosx
make install INSTALL_TOP=~/clib-prebuilt/macos

lldb --file ./dist/bin/cli