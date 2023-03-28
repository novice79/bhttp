#!/usr/bin/env bash
set -e
# set -x

scirptName=$0
dir=_build/android_*
example_prefix="dist/android"
lib_prefix=${prefix:-$example_prefix}
while [ $# -gt 0 ]; do
  case "$1" in
    -prefix=* | --PREFIX=* | prefix=* | PREFIX=*)
      lib_prefix="${1#*=}"
      ;;
    -h | --help)
      echo "build example app(s) and install lib to target dir with:"
      echo "$scirptName prefix=\$targetDir"
      echo "default lib installation dir: $example_prefix --if without prefix provided"
      echo "example app always install to $example_prefix "
      exit 0
      ;;
    *)
      printf "remove $dir for clean build\n"
      rm -rf $dir
  esac
  shift
done

# Set this to your minSdkVersion.
export API=23
abis=( 
    "armeabi-v7a"
    "arm64-v8a"
)
for abi in "${abis[@]}" ; do

IFS=':' read -ra ab <<< "$abi"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        PRBUILT="/data/clib-prebuilt/android/${ab[0]}"
elif [[ "$OSTYPE" == "darwin"* ]]; then
        PRBUILT="$HOME/clib-prebuilt/android/${ab[0]}"
elif [[ "$OSTYPE" == "win32" ]]; then
    :
else
    :
fi
dir="_build/android_${ab[0]}"

examplePrefix="$example_prefix/${ab[0]}"
PREFIX="$lib_prefix/${ab[0]}"
cmake -GNinja -H"bhttp" -B"$dir/lib" \
-DANDROID_ABI=${ab[0]} \
-DANDROID_NDK=$NDK \
-DCMAKE_C_FLAGS="-w" \
-DANDROID_NATIVE_API_LEVEL=$API \
-DANDROID_TOOLCHAIN=clang \
-DCMAKE_FIND_ROOT_PATH="${PRBUILT}" \
-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
-DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
-DCMAKE_BUILD_TYPE=Release 

cmake --build "$dir/lib"
cmake --install "$dir/lib"
# lib and app need to be same CFLAGS
# -DCMAKE_C_FLAGS="-w -DS_IREAD=S_IRUSR -DS_IWRITE=S_IWUSR -DS_IEXEC=S_IXUSR" \
# -DANDROID_STL=c++_static \  or c++_shared 
cmake -GNinja -H"examples" -B$dir \
-DANDROID_ABI=${ab[0]} \
-DANDROID_STL=c++_shared \
-DANDROID_NDK=$NDK \
-DCMAKE_C_FLAGS="-w" \
-DANDROID_NATIVE_API_LEVEL=$API \
-DANDROID_TOOLCHAIN=clang \
-DCMAKE_FIND_ROOT_PATH="${PRBUILT};${PREFIX}" \
-DCMAKE_INSTALL_PREFIX="$examplePrefix" \
-DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
-DCMAKE_BUILD_TYPE=Release 

cmake --build $dir --config Release
cmake --install $dir

done

# adb push dist/android/bin/app /data/local/tmp/