#!/bin/bash

: ${CMAKE_ANDROID_NDK?"Android NDK path must be set"}
: ${BUILD_ROOT? "Built roott path must be set"}

VERSION="1_69"

if [[ -z $ANDROID_ABI ]]; then
    echo "ANDROID_ABI not specified, using the default one: armeabi-v7a"
    ANDROID_ABI=armeabi-v7a
fi

git clone https://github.com/moritz-wundke/Boost-for-Android $BUILD_ROOT/d/boost

cd $BUILD_ROOT/d/boost

./build-android.sh --prefix=$BUILD_ROOT/i --with-libraries=system \
	--boost=1.69.0 --arch=$ANDROID_ABI \
	$CMAKE_ANDROID_NDK

cd $BUILD_ROOT/i/$ANDROID_ABI/lib

# possible only because just one library is being used
mv libboost_system-*-$VERSION.a libboost_system.a

