#! /bin/sh
set -e 

export scirptName=$0
case "$1" in
    -h | --help)
      echo "Usage: $scirptName prefix=$HOME/clib-prebuilt/linux"
      echo "default lib to $PWD/dist/linux"
      echo "example app always to $PWD/dist/linux/[arch]/bin"
      exit 0
      ;;
  esac
# $scirptName prefix=$HOME/clib-prebuilt/linux
$PWD/linux/armv7-ctng-linux-musleabihf.sh $@
$PWD/linux/aarch64-ctng-linux-musl.sh $@
$PWD/linux/x86_64-ctng-linux-musl.sh $@