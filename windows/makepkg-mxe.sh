#!/bin/bash

# Halt on errors
#set -e

# Be verbose
set -x

BUILDROOT=/data2/cross
MXEROOT=/data2/cross/mxe/usr/x86_64-w64-mingw32.shared
APP=krita

cd ${BUILDROOT}

VER=$(grep "#define CALLIGRA_VERSION_STRING" /build/libs/version/calligraversion.h | cut -d '"' -f 2)
cd ${BUILDROOT}/krita
BRANCH=$( git branch | cut -d ' ' -f 2)
REVISION=$(git rev-parse --short HEAD)
cd ..
VERSION=$VER$BRANCH-$REVISION
VERSION="$(sed s/\ /-/g <<<$VERSION)"
echo $VERSION

PACKAGENAME=$APP"-"$VERSION"-x64"

#rm -rf $BUILDROOT/out/* || true
mkdir -p $BUILDROOT/out/$PACKAGENAME
mkdir -p $BUILDROOT/out/$PACKAGENAME/bin
mkdir -p $BUILDROOT/out/$PACKAGENAME/lib
mkdir -p $BUILDROOT/out/$PACKAGENAME/share

cp $MXEROOT/bin/krita.exe $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/bin/*.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/bin/*.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/lib/libOpenColorIO.dll $BUILDROOT/out/$PACKAGENAME/bin
cp -r $MXEROOT/lib/plugins $BUILDROOT/out/$PACKAGENAME/bin
cp -r $MXEROOT/lib/kritaplugins $BUILDROOT/out/$PACKAGENAME/lib

cp $MXEROOT/qt5/bin/Qt5Concurrent.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Core.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Gui.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Network.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5OpenGL.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5PrintSupport.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Qml.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Quick.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Script.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5ScriptTools.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Svg.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5SystemInfo.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Widgets.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5WinExtras.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/qt5/bin/Qt5Xml.dll $BUILDROOT/out/$PACKAGENAME/bin

x86_64-w64-mingw32.shared-strip $BUILDROOT/out/$PACKAGENAME/bin/*

cp -r $MXEROOT/qt5/plugins/iconengines $BUILDROOT/out/$PACKAGENAME/bin/plugins
cp -r $MXEROOT/qt5/plugins/imageformats $BUILDROOT/out/$PACKAGENAME/bin/plugins
cp -r $MXEROOT/qt5/plugins/printsupport $BUILDROOT/out/$PACKAGENAME/bin/plugins
cp -r $MXEROOT/qt5/plugins/platforms $BUILDROOT/out/$PACKAGENAME/bin/

cp -r $MXEROOT/qt5/translations $BUILDROOT/out/$PACKAGENAME/bin/

cp -r $MXEROOT/share/color $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/color-schemes $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/kf5 $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/krita $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/locale $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/mime $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/ocio $BUILDROOT/out/$PACKAGENAME/share

rm krita-3.0-l10n-win-current.tar.gz || true
rm -rf locale

wget http://nonaynever.ru/pub/l10n-win/krita-3.0-l10n-win-current.tar.gz
tar -xf krita-3.0-l10n-win-current.tar.gz
mkdir $BUILDROOT/out/$PACKAGENAME/bin/data
cp -r $BUILDROOT/locale $BUILDROOT/out/$PACKAGENAME/bin/data

cd $BUILDROOT/out/

zip -r $PACKAGENAME.zip $PACKAGENAME
