#!/bin/bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#
#
# Build all Krita's dependencies on Ubuntu 14.04.
#
# Prerequisites: cmake git build-essential libxcb-keysyms1-dev plus all deps for Qt5
#

# Halt on errors and be verbose about what we are doing
set -e
set -x

# Read in our parameters
export BUILD_PREFIX=$1
export KRITA_SOURCES=$2

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment.
# That's not always the case, so make sure it is
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# We want to use $prefix/deps/usr/ for all our dependencies
export DEPS_INSTALL_PREFIX=$BUILD_PREFIX/deps/usr/
export DOWNLOADS_DIR=$BUILD_PREFIX/downloads/

# Setup variables needed to help everything find what we build
ARCH=`dpkg --print-architecture`
export LD_LIBRARY_PATH=$DEPS_INSTALL_PREFIX/lib:$LD_LIBRARY_PATH
export PATH=$DEPS_INSTALL_PREFIX/bin:$PATH
export PKG_CONFIG_PATH=$DEPS_INSTALL_PREFIX/share/pkgconfig:$DEPS_INSTALL_PREFIX/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$DEPS_INSTALL_PREFIX:$CMAKE_PREFIX_PATH

# A krita build layout looks like this:
# krita/ -- the source directory
# downloads/ -- downloads of the dependencies from files.kde.org
# deps-build/ -- build directory for the dependencies
# deps/ -- the location for the built dependencies
# build/ -- build directory for krita itself
# krita.appdir/ -- install directory for krita and the dependencies

# Make sure our downloads directory exists
if [ ! -d $DOWNLOADS_DIR ] ; then
    mkdir -p $DOWNLOADS_DIR
fi

# Make sure our build directory exists
if [ ! -d $BUILD_PREFIX/deps-build/ ] ; then
    mkdir -p $BUILD_PREFIX/deps-build/
fi

# The 3rdparty dependency handling in Krita also requires the install directory to be pre-created
if [ ! -d $DEPS_INSTALL_PREFIX ] ; then
    mkdir -p $DEPS_INSTALL_PREFIX
fi

# Switch to our build directory as we're basically ready to start building...
cd $BUILD_PREFIX/deps-build/


mkdir -p extra-build-tools
export PATH=$(pwd)/extra-build-tools:$PATH

# Our docker images are rather old, so we cannot use packaged versions for Meson
if ! meson --version > /dev/null 2>&1; then
    echo The Meson Build system was not found installing...
    MESON_VERSION=0.57.1
    wget -nc https://github.com/mesonbuild/meson/releases/download/$MESON_VERSION/meson-$MESON_VERSION.tar.gz
    tar xf meson-$MESON_VERSION.tar.gz
    mkdir -p $HOME/.local/bin
    ln -s $(pwd)/meson-$MESON_VERSION/meson.py $(pwd)/extra-build-tools/meson
    rm meson-$MESON_VERSION.tar.gz
fi

if ! ninja --version > /dev/null 2>&1; then
    echo The Ninja Build system was not found installing...
    NINJA_VERSION=1.10.2
    wget -nc https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/ninja-linux.zip
    unzip -o ninja-linux.zip
    ln -s $(pwd)/ninja $(pwd)/extra-build-tools/ninja
    rm ninja-linux.zip
fi

# Configure the dependencies for building
cmake $KRITA_SOURCES/3rdparty -DCMAKE_INSTALL_PREFIX=$DEPS_INSTALL_PREFIX -DINSTALL_ROOT=$DEPS_INSTALL_PREFIX -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOADS_DIR

# Now start building everything we need, in the appropriate order
#cmake --build . --config RelWithDebInfo --target ext_png
#cmake --build . --config RelWithDebInfo --target ext_tiff
#cmake --build . --config RelWithDebInfo --target ext_jpeg

cmake --build . --config RelWithDebInfo --target ext_boost
cmake --build . --config RelWithDebInfo --target ext_fftw3
cmake --build . --config RelWithDebInfo --target ext_eigen3
cmake --build . --config RelWithDebInfo --target ext_expat
cmake --build . --config RelWithDebInfo --target ext_exiv2
cmake --build . --config RelWithDebInfo --target ext_lcms2
cmake --build . --config RelWithDebInfo --target ext_ocio
cmake --build . --config RelWithDebInfo --target ext_openexr
if [[ $ARCH != "arm*" ]]; then
cmake --build . --config RelWithDebInfo --target ext_vc
fi
cmake --build . --config RelWithDebInfo --target ext_libraw
cmake --build . --config RelWithDebInfo --target ext_giflib
#cmake --build . --config RelWithDebInfo --target ext_gsl
cmake --build . --config RelWithDebInfo --target ext_python
#cmake --build . --config RelWithDebInfo --target ext_freetype
#cmake --build . --config RelWithDebInfo --target ext_fontconfig
cmake --build . --config RelWithDebInfo --target ext_qt
cmake --build . --config RelWithDebInfo --target ext_poppler
cmake --build . --config RelWithDebInfo --target ext_kcrash
cmake --build . --config RelWithDebInfo --target ext_gmic
cmake --build . --config RelWithDebInfo --target ext_sip
cmake --build . --config RelWithDebInfo --target ext_pyqt
cmake --build . --config RelWithDebInfo --target ext_quazip
cmake --build . --config RelWithDebInfo --target ext_openjpeg
cmake --build . --config RelWithDebInfo --target ext_nasm
cmake --build . --config RelWithDebInfo --target ext_libx265
cmake --build . --config RelWithDebInfo --target ext_libde265
cmake --build . --config RelWithDebInfo --target ext_libheif
cmake --build . --config RelWithDebInfo --target ext_seexpr
cmake --build . --config RelWithDebInfo --target ext_mypaint
cmake --build . --config RelWithDebInfo --target ext_fcitx-qt5
