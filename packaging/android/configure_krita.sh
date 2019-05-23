#!/bin/bash

: ${KRITA_ROOT?"Project root path must be set"}
: ${CMAKE_ANDROID_NDK?"Android NDK path must be set"}
: ${ANDROID_SDK_ROOT?"Android SDK path must be set"}
: ${ANDROID_API_LEVEL?"API level required"}
: ${QT_ANDROID?"path to qt android required"}
: ${INSTALL_PREFIX?"INSTALL_PREFIX must be set for building apk"}

CURDIR="$(pwd)"/


if [[ -z $ANDROID_ABI ]]; then
    echo "ANDROID_ABI not specified, using the default one: armeabi-v7a"
    ANDROID_ABI=armeabi-v7a
fi

ANDROID_NATIVE_API_LEVEL=android-$ANDROID_API_LEVEL

: ${PY_INCLUDE_PATH?"Python include path must be set"}
: ${PY_LIBRARY?"Python lib path must be set"}

PYTHON_INCLUDE_PATH=$PY_INCLUDE_PATH
PYTHON_LIBRARY=$PY_LIBRARY

# Configure files using cmake
cmake $KRITA_ROOT -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
     -DDEFINE_NO_DEPRECATED=1 \
     -DCMAKE_BUILD_TYPE=Debug \
     -DCMAKE_TOOLCHAIN_FILE=$CMAKE_ANDROID_NDK/build/cmake/android.toolchain.cmake \
     -DANDROID_PLATFORM=$ANDROID_NATIVE_API_LEVEL \
     -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_PATH \
     -DPYTHON_LIBRARY=$PYTHON_LIBRARY \
     -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3 \
     -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF \
     -DBoost_NO_BOOST_CMAKE=TRUE \
     -DBoost_NO_SYSTEM_PATHS=TRUE \
     -DQTANDROID_EXPORTED_TARGET=krita \
     -DANDROID_APK_DIR=$KRITA_ROOT/packaging/android/apk \
     -DANDROID_STL=c++_shared \
     -DANDROID_ABI=$ANDROID_ABI \
     -DCMAKE_FIND_ROOT_PATH="$QT_ANDROID;$BUILD_ROOT/kf5/kde/install/;$BUILD_ROOT/i"

