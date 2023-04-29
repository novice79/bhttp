#!/bin/sh
# set -x
set -e 

dir="$PWD/_build"
PREFIX=${prefix:-"$PWD/dist"}
while [ $# -gt 0 ]; do
  case "$1" in
    -prefix=* | --PREFIX=* | prefix=* | PREFIX=*)
      PREFIX="${1#*=}"
      ;;
    *)
      printf "remove $dir for clean build\n"
      rm -rf $dir
  esac
  shift
done
PRBUILT="$HOME/clib-prebuilt/macos"
# build http lib
cmake -GNinja -H"bhttp" -B"$dir/lib" \
-DCMAKE_FIND_ROOT_PATH="$PRBUILT" \
-DCMAKE_INSTALL_PREFIX=$PREFIX \
-DCMAKE_BUILD_TYPE=Release 

cmake --build "$dir/lib"
cmake --install "$dir/lib"

# build example exe
cmake -GNinja -H"examples" -B$dir \
-DCMAKE_FIND_ROOT_PATH="$PREFIX;$PRBUILT" \
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
printf "\nFor test:\n"
echo "run: ./dist/bin/app"
echo "and then use browser open http://localhost:8888/ to test spa"
echo "or use curl -X PUT ... to test restfull endpoint"
echo "and then run: ./dist/bin/cli to test client api"