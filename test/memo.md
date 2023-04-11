npm create vite@latest spa -- --template react

cd ~/cpp_libs/lua-5.4.4
make macosx
make install INSTALL_TOP=~/clib-prebuilt/macos

lldb --file ./dist/bin/cli
<!-- git diff file between branches -->
git diff ..master path/to/file

master.., i.e. the reverse of above. This is the same as master.
mybranch..master, explicitly referencing a state other than the current working tree.
v2.0.1..master, i.e., referencing a tag.
[refspec]..[refspec], basically anything identifiable as a code state to Git.

<!-- merge from another branch. some git commands memo -->
<!-- do not use checkout from another branch, cause it will auto staged
git checkout lua -- bhttp/client.hpp
git checkout lua -- bhttp/util.hpp
git checkout lua -- examples
git checkout lua -- Readme.md 
git checkout master -- test/spa/
-->
<!-- unstage all: git reset -->

<!-- get file from another branch without stage it -->
git restore --source lua -- build.sh
<!-- or -->
git show lua:bhttp/client_wss.hpp > bhttp/client_wss.hpp
git show lua:bhttp/client_https.hpp > bhttp/client_https.hpp
git diff ..lua -- bhttp/client_https.hpp

<!-- show node file size -->
echo $(($(wc -c $(which node) | awk '{ print $1 }')/1024/1024))M
ll $(realpath `which node`)