#!/bin/bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# Halt on errors and be verbose about what we are doing
set -e
set -x

# Read in our parameters
export BUILD_PREFIX=$1
export VERSION=2.4.2

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

# Switch to the build prefix
cd $BUILD_PREFIX

# G'Mic is built in build-deps.sh
# Therefore we just need to copy over the installation artifacts from that process
# First, make sure we have a clean slate to work from and setup the directory structure
rm -rf $BUILD_PREFIX/gmic_qt_krita.appdir/
mkdir -p $BUILD_PREFIX/gmic_qt_krita.appdir/usr/bin
mkdir -p $BUILD_PREFIX/gmic_qt_krita.appdir/usr/lib
mkdir -p $BUILD_PREFIX/gmic_qt_krita.appdir/usr/share

# Copy over the artifacts...
cp $DEPS_INSTALL_PREFIX/bin/gmic_krita_qt* $BUILD_PREFIX/gmic_qt_krita.appdir/usr/bin
cp $BUILD_PREFIX/deps-build/ext_gmic/gmic-qt/resources/gmic_hat.png $BUILD_PREFIX/gmic_qt_krita.appdir/gmic_krita_qt.png

# Now generate the Appimage for it
linuxdeployqt $BUILD_PREFIX/gmic_qt_krita.appdir/usr/bin/gmic_krita_qt.desktop -verbose=2 -bundle-non-qt-libs -appimage

# Make sure it has a consistent name too
if [[ $ARCH == "arm64" ]]; then
  APPIMAGE_ARCHITECTURE="aarch64"
elif [[ $ARCH == "amd64" ]]; then
  APPIMAGE_ARCHITECTURE="x86_64"
else
  APPIMAGE_ARCHITECTURE=$ARCH
fi

mv gmic_krita_qt*$APPIMAGE_ARCHITECTURE.AppImage gmic_krita_qt-$APPIMAGE_ARCHITECTURE.appimage
