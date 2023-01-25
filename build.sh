#!/bin/sh
# set -x
set -e 

dir="_build"
[[ "$#" -gt 0 ]] && rm -rf $dir

# build http lib
cmake -GNinja -H"bhttp" -B"$dir/lib" \
-DCMAKE_FIND_ROOT_PATH="$HOME/clib-prebuilt/macos" \
-DCMAKE_INSTALL_PREFIX=dist \
-DCMAKE_BUILD_TYPE=Release 

cmake --build "$dir/lib"
cmake --install "$dir/lib"

# build example exe
cmake -GNinja -H"examples" -B$dir \
-DCMAKE_FIND_ROOT_PATH="$HOME/clib-prebuilt/macos;$PWD/dist" \
-DCMAKE_INSTALL_PREFIX=dist \
-DCMAKE_BUILD_TYPE=Release 

cmake --build $dir
cmake --install $dir

# write static test files
mkdir -p $PWD/dist/bin/{www,store}
# echo "hello world from www dir static index.html file" > $PWD/dist/bin/www/index.html
echo "hello world from store dir static index.html file" > $PWD/dist/bin/store/index.html
# cd test && npm create vite@latest spa -- --template react
cd test/spa && npm i && npm run build
cd -
# run example exe
printf "\nFYI:\n"
echo "please run: ./dist/bin/app"
echo "and then use browser open http://localhost:8888/ to test spa"
echo "or use curl -X PUT ... to test restfull endpoint"
echo "and then run: ./dist/bin/cli to test client api"