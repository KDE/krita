#!/bin/sh

: ${CMAKE_ANDROID_NDK?"Android NDK path must be set"}
: ${BUILD_ROOT? "Built roott path must be set"}

CURDIR="$(pwd)"/
VERSION="1_69"

git clone https://github.com/moritz-wundke/Boost-for-Android $BUILD_ROOT/d/boost

cd $BUILD_ROOT/d/boost

./build-android.sh --prefix=$BUILD_ROOT/i --with-libraries=system \
	--boost=1.69.0 --arch=armeabi-v7a \
	$CMAKE_ANDROID_NDK

cd $BUILD_ROOT/i/armeabi-v7a/lib

# possible only because just one library is being used
mv libboost_system-clang-mt-a32-$VERSION.a libboost_system.a

