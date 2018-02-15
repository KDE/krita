# Halt on errors
set -e

# Be verbose
set -x

export BUILD_PREFIX=/home/krita/devel
export INSTALLDIR=$BUILD_PREFIX/krita.appdir/usr
export QTDIR=$BUILD_PREFIX/deps/usr
export LD_LIBRARY_PATH=$QTDIR/sip:$QTDIR/lib/x86_64-linux-gnu:$QTDIR/lib:$BUILD_PREFIX/i/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR/bin:$BUILD_PREFIX/i/bin:$PATH
export PKG_CONFIG_PATH=$QTDIR/share/pkgconfig:$QTDIR/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$QTDIR/lib/x86_64-linux-gnu:$BUILD_PREFIX:$QTDIR:$CMAKE_PREFIX_PATH

export PYTHONPATH=$QTDIR/sip:$QTDIR/lib/python3.5/site-packages:$QTDIR/lib/python3.5
export PYTHONHOME=$QTDIR

export APPDIR=$BUILD_PREFIX/krita.appdir
export PLUGINS=$APPDIR/usr/lib/kritaplugins/

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment. That's
# not always set correctly in CentOS 6.7
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

rm -rf $APPDIR
cd $BUILD_PREFIX/krita_build
make -j10 install
cd  -

cp -r $QTDIR/share/kf5 $APPDIR/usr/share
cp -r $QTDIR/share/locale $APPDIR/usr/share
cp -r $QTDIR/share/mime $APPDIR/usr/share
cp -r $QTDIR/lib/python3.5 $APPDIR/usr/lib
cp -r $QTDIR/sip $APPDIR/usr/lib/

mv $APPDIR/usr/lib/x86_64-linux-gnu/*  $APPDIR/usr/lib
rm -rf $APPDIR/usr/lib/x86_64-linux-gnu/

for lib in $PLUGINS/*.so*; do
  patchelf --set-rpath '$ORIGIN/..' $lib; 
done

for lib in $APPDIR/usr/lib/python3.5/site-packages/PyQt5/*.so*; do
  patchelf --set-rpath '$ORIGIN/../..' $lib; 
done

for lib in $APPDIR/usr/lib/python3.5/lib-dynload/*.so*; do
  patchelf --set-rpath '$ORIGIN/../..' $lib; 
done

patchelf --set-rpath '$ORIGIN/../../../..' $APPDIR/usr/lib/qml/org/krita/draganddrop/libdraganddropplugin.so
patchelf --set-rpath '$ORIGIN/../../../..' $APPDIR/usr/lib/qml/org/krita/sketch/libkritasketchplugin.so
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/sip/sip.so

#
# Get the latest linuxdeployqt
#
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
chmod a+x linuxdeployqt

./linuxdeployqt $APPDIR/usr/share/applications/org.kde.krita.desktop \
  -executable=$APPDIR/usr/bin/krita \
  -qmldir=$QTDIR/qml \
  -verbose=2 \
  -bundle-non-qt-libs \
  -extra-plugins=$PLUGINS,$APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so,$APPDIR/usr/lib//qml/org/krita/sketch/libkritasketchplugin.so,$APPDIR/usr/lib/qml/org/krita/draganddrop/libdraganddropplugin.so  \
  -appimage 

  
cd $BUILD_PREFIX/krita_build
VER=$(grep "#define KRITA_VERSION_STRING" libs/version/kritaversion.h | cut -d '"' -f 2)
cd -
cd $BUILD_PREFIX/krita
REVISION=$(git rev-parse --short HEAD)
cd -

VERSION=$VER-$REVISION
VERSION="$(sed s/\ /-/g <<<$VERSION)"
echo $VERSION


# Determine which architecture should be built
if [[ "$(arch)" = "i686" || "$(arch)" = "x86_64" ]] ; then
  ARCH=$(arch)
else
  echo "Architecture could not be determined"
  exit 1
fi

if [[ "$ARCH" = "x86_64" ]] ; then
        APPIMAGE=krita-"$VERSION"-x86_64.appimage"
fi
if [[ "$ARCH" = "i686" ]] ; then
        APPIMAGE=krita-"$VERSION"-i386.appimage"
fi
echo $APPIMAGE

mv Krita-x86_64.AppImage $APPIMAGE


