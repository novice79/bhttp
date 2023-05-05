#! /bin/sh

scirptName=$0
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        PRBUILT="/data/clib-prebuilt/ios"
elif [[ "$OSTYPE" == "darwin"* ]]; then
        PRBUILT="$HOME/clib-prebuilt/ios"
elif [[ "$OSTYPE" == "win32" ]]; then
    :
else
    :
fi
dir="_build/ios"
examplePrefix="dist/ios"
PREFIX=${prefix:-$examplePrefix}
while [ $# -gt 0 ]; do
  case "$1" in
    -prefix=* | --PREFIX=* | prefix=* | PREFIX=*)
      PREFIX="${1#*=}"
      ;;
    lib)
      LIB_ONLY=true
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
cmake -H"bhttp" -B"$dir/lib" \
-G Xcode \
-DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/ios.toolchain.cmake" \
-DDEPLOYMENT_TARGET=13.0 \
-DPLATFORM=OS64 \
-DCMAKE_FIND_ROOT_PATH="$PRBUILT" \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=$PREFIX

cmake --build "$dir/lib" --config Release -- -allowProvisioningUpdates
cmake --install "$dir/lib"

[ "$LIB_ONLY" = true ] && exit 0
cmake -H"examples" -B"$dir" \
-G Xcode \
-DCMAKE_TOOLCHAIN_FILE="$PWD/cmake/ios.toolchain.cmake" \
-DDEPLOYMENT_TARGET=13.0 \
-DPLATFORM=OS64 \
-DCMAKE_FIND_ROOT_PATH="${PRBUILT};${PREFIX}" \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=$examplePrefix
# grep DEVELOPMENT_TEAM _build_ios/main.xcodeproj/project.pbxproj
cmake --build $dir --config Release -- -allowProvisioningUpdates
cmake --install $dir