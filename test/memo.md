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