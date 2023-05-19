#!/usr/bin/env bash
#  ./ios_build.sh prefix=$HOME/clib-prebuilt/ios
set -e
scirptName=$0
ToolChainFile=""$PWD/cmake/ios.toolchain.cmake""
PRBUILT="$HOME/clib-prebuilt/ios"
PREFIX="$PWD/dist/ios"
rm -rf "$PREFIX"
prefix="$PREFIX"
while [ $# -gt 0 ]; do
  case "$1" in
    -prefix=* | --PREFIX=* | prefix=* | PREFIX=*)
      prefix="${1#*=}"
      ;;
    lib)
      LIB_ONLY=true
      ;;
    -h | --help)
      echo "build example app(s) and install lib to target dir with:"
      echo "$scirptName prefix=\$targetDir"
      echo "default lib installation dir: $PREFIX --if without prefix provided"
      echo "example app always install to $PREFIX "
      exit 0
      ;;
    *)
      printf "remove $dir for clean build\n"
      rm -rf $dir
  esac
  shift
done
# CMAKE_PREFIX_PATH vs CMAKE_FIND_ROOT_PATH
platforms=( 
    "OS64:dev-arm64"
    "SIMULATORARM64:sim-arm64"
)
# pa for platform-arch
for pa in "${platforms[@]}" ; do
# paa for pa array
IFS=':' read -ra paa <<< "$pa"

dir="$PWD/_build/${paa[1]}"
cmake -H"bhttp" -B"$dir/lib" \
-G Xcode \
-DCMAKE_TOOLCHAIN_FILE="$ToolChainFile" \
-DDEPLOYMENT_TARGET=13.0 \
-DENABLE_BITCODE=OFF \
-DPLATFORM="${paa[0]}" \
-DCMAKE_FIND_ROOT_PATH="$PRBUILT/${paa[1]}" \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX="$PREFIX/${paa[1]}"

cmake --build "$dir/lib" --config Release -- -allowProvisioningUpdates
cmake --install "$dir/lib"

[[ $PREFIX != $prefix ]] && cp -rv "$PREFIX/${paa[1]}"/* "$prefix/${paa[1]}"
$(xcrun --find --sdk iphoneos libtool) -static -o "$PREFIX/${paa[1]}/lib/libbhttp-${paa[1]}.a" "$PREFIX/${paa[1]}"/lib/*.a
[ "$LIB_ONLY" = true ] && continue
cmake -H"examples" -B"$dir" \
-G Xcode \
-DCMAKE_TOOLCHAIN_FILE="$ToolChainFile" \
-DDEPLOYMENT_TARGET=13.0 \
-DENABLE_BITCODE=OFF \
-DPLATFORM="${paa[0]}" \
-DCMAKE_FIND_ROOT_PATH="${PREFIX};${PRBUILT}/${paa[1]}" \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX="$PREFIX/${paa[1]}"
# grep DEVELOPMENT_TEAM _build_ios/main.xcodeproj/project.pbxproj
cmake --build $dir --config Release -- -allowProvisioningUpdates
cmake --install $dir

done
rm -rfv "$prefix/bhttp.xcframework"
xcodebuild -create-xcframework \
-library $PREFIX/dev-arm64/lib/libbhttp-dev-arm64.a \
-headers $PREFIX/dev-arm64/include \
-library $PREFIX/sim-arm64/lib/libbhttp-sim-arm64.a \
-headers $PREFIX/sim-arm64/include \
-output "$prefix/bhttp.xcframework"