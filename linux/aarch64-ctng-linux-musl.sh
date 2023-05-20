#!/bin/bash

set -e 

# brew tap novice79/gcc-cross
# brew install armv7-ctng-linux-musleabihf aarch64-ctng-linux-musl x86_64-ctng-linux-musl
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        PRBUILT="/cross/aarch64-ctng-linux-musl"
        TOOLCHAIN_FILE="/toolchains/aarch64-ctng-linux-musl/toolchain.cmake"
elif [[ "$OSTYPE" == "darwin"* ]]; then
        PRBUILT="$HOME/clib-prebuilt/linux/aarch64"
        TOOLCHAIN_FILE="$(brew --prefix aarch64-ctng-linux-musl)/toolchain.cmake"
elif [[ "$OSTYPE" == "win32" ]]; then
    :
else
    :
fi

examplePrefix="$PWD/dist/linux/aarch64"
PREFIX=${prefix:-$examplePrefix}
dir="$PWD/_build/linux/aarch64"

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
PREFIX="${PREFIX%/aarch64}/aarch64"

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
