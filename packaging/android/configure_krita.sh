#!/bin/sh

: ${KRITA_ROOT?"Project root path must be set"}
: ${CMAKE_ANDROID_NDK?"Android NDK path must be set"}
: ${ANDROID_SDK_ROOT?"Android SDK path must be set"}
: ${ANDROID_API_LEVEL?"API level required"}
: ${QT_ANDROID?"path to qt android required"}

CURDIR="$(pwd)"/

export ANDROID_ARCHITECTURE=arm
export ANDROID_ABI=armeabi-v7a
export ANDROID_TOOLCHAIN=arm-linux-androideabi
export ANDROID_NATIVE_API_LEVEL=android-$ANDROID_API_LEVEL


: ${PY_INCLUDE_PATH?"Python include path must be set"}
: ${PY_LIBRARY?"Python lib path must be set"}

PYTHON_INCLUDE_PATH=$PY_INCLUDE_PATH
PYTHON_LIBRARY=$PY_LIBRARY

# Configure files using cmake
cmake $KRITA_ROOT -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
	 -DDEFINE_NO_DEPRECATED=1 \
	 -DBUILD_TESTING=OFF \
	 -DKDE4_BUILD_TESTS=OFF \
	 -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	 -DCMAKE_TOOLCHAIN_FILE=$CMAKE_ANDROID_NDK/build/cmake/android.toolchain.cmake \
	 -DANDROID_PLATFORM=$ANDROID_NATIVE_API_LEVEL \
	 -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_PATH \
	 -DPYTHON_LIBRARY=$PYTHON_LIBRARY \
	 -DCMAKE_PREFIX_PATH=$QT_ANDROID \
	 -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3 \
	 -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF \
	 -DBoost_NO_BOOST_CMAKE=TRUE \
	 -DBoost_NO_SYSTEM_PATHS=TRUE


