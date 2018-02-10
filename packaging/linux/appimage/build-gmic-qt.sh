#!/bin/bash

# Be verbose
# Halt on errors
set -e

# Be verbose
set -x

export BUILD_PREFIX=/media/krita/_devel
export QTDIR=$BUILD_PREFIX/deps/usr
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR=bin:$PATH
export PKG_CONFIG_PATH=$QTDIR/share/pkgconfig:$QTDIR/lib/pkgconfig:$BUILD_PREFIX/i/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$BUILD_PREFIX:$QTDIR:$CMAKE_PREFIX_PATH

#
# G'Mic is built in build-deps.sh
#
rm -rf $BUILD_PREFIX/app/
mkdir -p $BUILD_PREFIX/app/usr/bin
cp $QTDIR/bin/gmic_krita_qt* $BUILD_PREFIX/app/usr/bin
mkdir -p $BUILD_PREFIX/app/usr/lib
mkdir -p $BUILD_PREFIX/app/usr/share

#
# Get the latest linuxdeployqt
#
cd
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
chmod a+x linuxdeployqt
./linuxdeployqt $BUILD_PREFIX/app/usr/bin/gmic_krita_qt.desktop -verbose=2 -bundle-non-qt-libs -appimage

#mv *AppImage /krita.appdir/usr/bin/gmic_qt_krita
