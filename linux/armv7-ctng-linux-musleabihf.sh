#!/bin/bash

set -e 
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        PRBUILT="/cross/armv7-ctng-linux-musleabihf"
        TOOLCHAIN_FILE="/toolchains/armv7-ctng-linux-musleabihf/toolchain.cmake"
elif [[ "$OSTYPE" == "darwin"* ]]; then
        PRBUILT="$HOME/clib-prebuilt/linux/armv7"
        TOOLCHAIN_FILE="$(brew --prefix armv7-ctng-linux-musleabihf)/toolchain.cmake"
elif [[ "$OSTYPE" == "win32" ]]; then
    :
else
    :
fi

examplePrefix="$PWD/dist/linux/armv7"
PREFIX=${prefix:-$examplePrefix}
dir="$PWD/_build/linux/armv7"

while [ $# -gt 0 ]; do
  case "$1" in
    -prefix=* | --PREFIX=* | prefix=* | PREFIX=*)
      PREFIX="${1#*=}"
      ;;
    lib)
      LIB_ONLY=true
      ;;
    *)
      printf "remove $dir for clean build\n"
      rm -rf $dir
  esac
  shift
done

PREFIX="${PREFIX%/armv7}/armv7"
cmake -H"bhttp" -B"$dir/lib" \
-G Ninja \
-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
-DCMAKE_FIND_ROOT_PATH=$PRBUILT \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=$PREFIX


cmake --build "$dir/lib" --config Release
cmake --install $dir/lib
[ "$LIB_ONLY" = true ] && exit 0
# build example exe
cmake -GNinja -H"examples" -B$dir \
-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
-DCMAKE_FIND_ROOT_PATH="$PREFIX;$PRBUILT" \
-DCMAKE_INSTALL_PREFIX=$examplePrefix \
-DCMAKE_BUILD_TYPE=Release 

cmake --build $dir
cmake --install $dir

# strip $examplePrefix/app
ls -lh $examplePrefix/bin
file $examplePrefix/bin/app
