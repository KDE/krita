#!/bin/bash

# parts used from KDE/KStars project

: ${QT_ANDROID?"Qt path must be set"}
: ${CMAKE_ANDROID_NDK?"Android NDK path must be set"}
: ${ANDROID_SDK_ROOT?"Android SDK path must be set"}
: ${ANDROID_API_LEVEL?"Android API level"}
: ${KRITA_ROOT?"Project root path must be set"}

export ANDROID_ARCHITECTURE=arm
export ANDROID_ABI=armeabi-v7a
export ANDROID_TOOLCHAIN=arm-linux-androideabi
export ANDROID_NATIVE_API_LEVEL=android-$ANDROID_API_LEVEL

cd $KRITA_ROOT


 # Get the directory where the script is stored
SRCDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CURDIR="$(pwd)"/


mkdir kf5
cd kf5
git clone git://anongit.kde.org/scratch/cordlandwehr/kdesrc-conf-android.git
mkdir -p extragear/kdesrc-build
git clone git://anongit.kde.org/kdesrc-build extragear/kdesrc-build
ln -s extragear/kdesrc-build/kdesrc-build kdesrc-build
ln -s kdesrc-conf-android/kdesrc-buildrc kdesrc-buildrc


# Change the build configuration
sed -E -i "s|build-dir.*|build-dir $CURDIR/kf5/kde/build/${android_architecture} |g" kdesrc-conf-android/kdesrc-buildrc
sed -E -i "s|source-dir.*|source-dir $CURDIR/kf5/kde/src |g" kdesrc-conf-android/kdesrc-buildrc
sed -E -i "s|kdedir.*|kdedir $CURDIR/kf5/kde/install/${android_architecture} |g" kdesrc-conf-android/kdesrc-buildrc
sed -i -- 's/make-options -j8/make-options -j4 VERBOSE=1/g' kdesrc-conf-android/kdesrc-buildrc

# The toolchain provided by Linux distribution can be old, use this instead
./kdesrc-build extra-cmake-modules

if [ -e $qt_android_libs ]
then
    sed -E -i "s|-DCMAKE_PREFIX_PATH=.*?\\ |-DCMAKE_PREFIX_PATH=$QT_ANDROID- -DCMAKE_ANDROID_NDK=$CMAKE_ANDROID_NDK -DECM_ADDITIONAL_FIND_ROOT_PATH=$QT_ANDROID\;$CURDIR/kf5/kde/install -DANDROID_STL=c++_static -DCMAKE_TOOLCHAIN_FILE=$CURDIR/kf5/kde/install/share/ECM/toolchain/Android.cmake -DKCONFIG_USE_DBUS=OFF |g" kdesrc-conf-android/kdesrc-buildrc
else
    echo "Qt Android libraries path doesn't exist. Exiting."
    exit
fi


sed -E -i "s|use-modules.+|use-modules kconfig ki18n |g" kdesrc-conf-android/kdesrc-buildrc
rm -rf ${kf5_android_path}/kde/build/${android_architecture}/* # clean build folders


# Needs to be patched, ignore the errors
./kdesrc-build libintl-lite ki18n kcoreaddons kcompletion kitemmodels


# patches
sed -i -- 's/check_cxx_source_compiles/#check_cxx_source_compiles/g' kde/src/frameworks/ki18n/cmake/FindLibIntl.cmake
sed -i -- 's/target_link_libraries(ktranscript PRIVATE Qt5::Qml Qt5::Core)/target_link_libraries(ktranscript PRIVATE Qt5::Qml Qt5::Core -l:libc.a -Wl,--exclude-libs=ALL)/g' $CURDIR/kf5/kde/src/frameworks/ki18n/src/CMakeLists.txt

sed -i 's/static \(.*getpwent()\)/\1/' $CURDIR/kf5/kde/src/frameworks/kcoreaddons/src/lib/util/kuser_unix.cpp
sed -i 's/static \(.*setgrent()\)/\1/' $CURDIR/kf5/kde/src/frameworks/kcoreaddons/src/lib/util/kuser_unix.cpp
sed -i 's/static \(.*getgrent()\)/\1/' $CURDIR/kf5/kde/src/frameworks/kcoreaddons/src/lib/util/kuser_unix.cpp
sed -i 's/static \(.*endgrent()\)/\1/' $CURDIR/kf5/kde/src/frameworks/kcoreaddons/src/lib/util/kuser_unix.cpp

sed -i 's/KF5::WidgetsAddons/& -l:libc.a -Wl,--exclude-libs=ALL/' $CURDIR/kf5/kde/src/frameworks/kcompletion/src/CMakeLists.txt

sed -i '/add_subdirectory(tests)/s/^/#/' $CURDIR/kf5/kde/src/frameworks/kitemmodels/CMakeLists.txt
sed -i '/add_subdirectory(autotests)/s/^/#/' $CURDIR/kf5/kde/src/frameworks/kitemmodels/CMakeLists.txt



./kdesrc-build kcoreaddons \
     frameworks-android \
     kwidgetsaddons kcompletion \
     kguiaddons ki18n \
     kitemmodels kitemviews \
     kwindowsystem

# Fix some config files
sed -i '/find_package(PythonInterp/ s/^/#/' kde/install/lib/cmake/KF5I18n/KF5I18nMacros.cmake

cp /usr/lib/x86_64-linux-gnu/libexec/kf5/kconfig_compiler_kf5 $CURDIR/kf5/kde/install/lib/libexec/kf5/kconfig_compiler_kf5

cd $CURDIR

