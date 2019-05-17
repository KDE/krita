#!/bin/bash -e

# See more: 3rdparty/README.md
# Build external dependencies in $KRITA_ROOT/3rdparty

# assuming in $KRITA_ROOT/b

: ${KRITA_ROOT?"Project root path must be set"}
: ${CMAKE_ANDROID_NDK?"Android NDK path must be set"}
: ${ANDROID_API_LEVEL?"Android API level is required"}
: ${QT_ANDROID?"Path to QT root is required"}
: ${BUILD_ROOT? "Build root must be set"}

if [[ -z $ANDROID_ABI ]]; then
    echo "ANDROID_ABI not specified, using the default one: armeabi-v7a"
    ANDROID_ABI=armeabi-v7a
fi

ANDROID_NATIVE_API_LEVEL=android-$ANDROID_API_LEVEL

cmake $KRITA_ROOT/3rdparty \
    -DINSTALL_ROOT=$BUILD_ROOT/i \
    -DEXTERNALS_DOWNLOAD_DIR=$BUILD_ROOT/d \
    -DCMAKE_INSTALL_PREFIX=$BUILD_ROOT/i \
    -DCMAKE_TOOLCHAIN_FILE=$CMAKE_ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_PLATFORM=$ANDROID_NATIVE_API_LEVEL \
    -DANDROID_ABI=$ANDROID_ABI \
    -DANDROID_STL=c++_static


# You can comment these and build them individually
cmake --build . --config RelWithDebInfo --target ext_png
cmake --build . --config RelWithDebInfo --target ext_zlib
cmake --build . --config RelWithDebInfo --target ext_quazip
cmake --build . --config RelWithDebInfo --target ext_lcms2
# this one SHOULD be built before exiv
cmake --build . --config RelWithDebInfo --target ext_expat
cmake --build . --config RelWithDebInfo --target ext_exiv2
cmake --build . --config RelWithDebInfo --target ext_gsl

