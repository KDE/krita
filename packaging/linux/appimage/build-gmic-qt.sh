#!/bin/bash

# Enter a CentOS 6 chroot (you could use other methods)
# git clone https://github.com/probonopd/AppImageKit.git
# ./AppImageKit/build.sh 
# sudo ./AppImageKit/AppImageAssistant.AppDir/testappimage /isodevice/boot/iso/CentOS-6.5-x86_64-LiveCD.iso bash

# Halt on errors
set -e

# Be verbose
set -x

# Now we are inside CentOS 6
grep -r "CentOS release 6" /etc/redhat-release || exit 1

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment. That's
# not always set correctly in CentOS 6.7
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# Determine which architecture should be built
if [[ "$(arch)" = "i686" || "$(arch)" = "x86_64" ]] ; then
  ARCH=$(arch)
else
  echo "Architecture could not be determined"
  exit 1
fi

# Use the new compiler
. /opt/rh/devtoolset-3/enable


# Workaround for: On CentOS 6, .pc files in /usr/lib/pkgconfig are not recognized
# However, this is where .pc files get installed when building libraries... (FIXME)
# I found this by comparing the output of librevenge's "make install" command
# between Ubuntu and CentOS 6
ln -sf /usr/share/pkgconfig /usr/lib/pkgconfig

cd /

# Get gmic
rm -rf gmic
git clone https://github.com/dtschump/gmic.git
make -C gmic/src CImg.h gmic_stdlib.h

# Get gmic-qt
make -C gmic/src CImg.h gmic_stdlib.h
cd gmic-qt
mkdir build 
cd build
cmake .. -DGMIC_QT_HOST=krita -DCMAKE_BUILD_TYPE=Release

wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
./linuxdeployqt-continuous-x86_64.AppImage gmic_qt_krita -verbose=2 -appimage -bundle-non-qt-libs

mv *AppImage /krita.appdir/usr/bin/gmic_qt_krita
