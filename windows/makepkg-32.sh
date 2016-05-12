#!/bin/bash

# Halt on errors
set -e

# Be verbose
set -x

BUILDROOT=/data2/cross
MXEROOT=/data2/cross/mxe/usr/i686-w64-mingw32.shared
APP=krita

cd $BUILDROOT

VER=$(grep "#define KRITA_VERSION_STRING" $BUILDROOT/build/libs/version/kritaversion.h | cut -d '"' -f 2)
cd $BUILDROOT/krita
BRANCH=$( git branch | cut -d ' ' -f 2)
REVISION=$(git rev-parse --short HEAD)
cd ..
VERSION=$VER$BRANCH-$REVISION
VERSION="$(sed s/\ /-/g <<<$VERSION)"
echo $VERSION

PACKAGENAME=$APP"-"$VERSION"-x86"

mkdir -p $BUILDROOT/out/$PACKAGENAME
mkdir -p $BUILDROOT/out/$PACKAGENAME/bin/data
mkdir -p $BUILDROOT/out/$PACKAGENAME/lib
mkdir -p $BUILDROOT/out/$PACKAGENAME/share

cp $BUILDROOT/krita/windows/krita.lnk  $BUILDROOT/out/$PACKAGENAME
cp $BUILDROOT/krita/windows/qt.conf  $BUILDROOT/out/$PACKAGENAME/bin

cp $MXEROOT/bin/krita.exe $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/bin/*.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/bin/*.dll $BUILDROOT/out/$PACKAGENAME/bin
cp $MXEROOT/lib/libOpenColorIO.dll $BUILDROOT/out/$PACKAGENAME/bin
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

cp -r $MXEROOT/qt5/plugins/iconengines $BUILDROOT/out/$PACKAGENAME/bin/
cp -r $MXEROOT/qt5/plugins/imageformats $BUILDROOT/out/$PACKAGENAME/bin/
cp -r $MXEROOT/qt5/plugins/printsupport $BUILDROOT/out/$PACKAGENAME/bin/
cp -r $MXEROOT/qt5/plugins/platforms $BUILDROOT/out/$PACKAGENAME/bin/
cp -r $MXEROOT/lib/plugins/imageformats/* $BUILDROOT/out/$PACKAGENAME/bin/imageformats/

mkdir $BUILDROOT/out/$PACKAGENAME/bin/translations
cp -r $MXEROOT/qt5/translations/qt_* $BUILDROOT/out/$PACKAGENAME/bin/translations
cp -r $MXEROOT/qt5/translations/qtbase* $BUILDROOT/out/$PACKAGENAME/bin/translations

cp -r $MXEROOT/share/color $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/color-schemes $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/kf5 $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/krita $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/locale $BUILDROOT/out/$PACKAGENAME/bin/data
cp -r $MXEROOT/share/mime $BUILDROOT/out/$PACKAGENAME/share
cp -r $MXEROOT/share/ocio $BUILDROOT/out/$PACKAGENAME/share

cd $BUILDROOT
rm krita-3.0-l10n-win-current.tar.gz || true
rm -rf locale

wget http://files.kde.org/krita/build/krita-3.0-l10n-win-current.tar.gz
tar -xf krita-3.0-l10n-win-current.tar.gz
cp -r $BUILDROOT/locale $BUILDROOT/out/$PACKAGENAME/bin/data

cd $BUILDROOT/out/

zip -r $PACKAGENAME-dbg.zip $PACKAGENAME

find $BUILDROOT/out/$PACKAGENAME/bin -name \*exe | xargs $BUILDROOT/mxe/usr/bin/i686-w64-mingw32.shared-strip
find $BUILDROOT/out/$PACKAGENAME/bin -name \*dll | xargs $BUILDROOT/mxe/usr/bin/i686-w64-mingw32.shared-strip
find $BUILDROOT/out/$PACKAGENAME/lib -name \*dll | xargs $BUILDROOT/mxe/usr/bin/i686-w64-mingw32.shared-strip

zip -r $PACKAGENAME.zip $PACKAGENAME
