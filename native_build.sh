#!/bin/sh
# set -x
set -e 
# native build means build on x86_64 macos or linux
scirptName=$0

# $OSTYPE not exist on alpine busybox, so use more reliable uname methond
ostype=$(uname)
if [[ "$ostype" == "Linux"* ]]; then
    PRBUILT="/data/clib-prebuilt/linux/x86_64"
elif [[ "$ostype" == "Darwin"* ]]; then
    PRBUILT="$HOME/clib-prebuilt/macos"
else
    :
fi
dir="$PWD/_build"
examplePrefix="$PWD/dist"
PREFIX=${prefix:-$examplePrefix}
while [ $# -gt 0 ]; do
  case "$1" in
    -prefix=* | --PREFIX=* | prefix=* | PREFIX=*)
      PREFIX="${1#*=}"
      ;;
    -h | --help)
      echo "build example app(s) and install lib to target dir with:"
      echo "$scirptName prefix=\$targetDir"
      echo "default lib installation dir: $examplePrefix --if without prefix provided"
      echo "example app always install to $examplePrefix "
      exit 0
      ;;
    *)
      printf "remove $dir for clean build\n"
      rm -rf $dir
  esac
  shift
done

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
-DCMAKE_INSTALL_PREFIX=$examplePrefix \
-DCMAKE_BUILD_TYPE=Release 

cmake --build $dir
cmake --install $dir

# write static test files
mkdir -p "$PWD/dist/bin"/{www,store}
# echo "hello world from www dir static index.html file" > $PWD/dist/bin/www/index.html
echo "hello world from store dir static index.html file" > $PWD/dist/bin/store/index.html
# cd test && npm create vite@latest spa -- --template react
cd test/spa && npm i && npm run build
cd -
# run example exe
printf "\nFYI:\n"
echo "please run: ./dist/bin/app"
echo "and then use browser open http://localhost:8888/ or https://127.0.0.1:9999/ to test spa app"
echo "or use curl -X PUT ... to test restfull endpoint"
echo "and then run: ./dist/bin/cli to test client api"