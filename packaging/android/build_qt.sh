#!/bin/bash

: ${KRITA_ROOT?"Project root path must be set"}
: ${CMAKE_ANDROID_NDK?"Android NDK path must be set"}
: ${BUILD_ROOT? "Build root must be set"}
: ${ANDROID_SDK_ROOT? "Android SDK path must be set"}

if [[ -z $ANDROID_ABI ]]; then
    echo "ANDROID_ABI not specified, using the default one: armeabi-v7a"
    ANDROID_ABI=armeabi-v7a
fi

cmake $KRITA_ROOT/3rdparty \
    -DINSTALL_ROOT=$BUILD_ROOT/i \
    -DEXTERNALS_DOWNLOAD_DIR=$BUILD_ROOT/d \
    -DCMAKE_INSTALL_PREFIX=$BUILD_ROOT/i \
    -DCMAKE_TOOLCHAIN_FILE=$CMAKE_ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ANDROID_ABI \
    -DANDROID_STL=c++_static \
    -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT

cmake --build . --config RelWithDebInfo --target ext_qt

