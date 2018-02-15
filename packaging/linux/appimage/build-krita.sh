#!/bin/bash

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

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment. That's
# not always set correctly in CentOS 6.7
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

git_pull_rebase_helper()
{
	git reset --hard HEAD
	git pull --rebase
}

# A krita build layout looks like this:
# krita/ -- the source directory
# krita/3rdparty -- the cmake3 definitions for the dependencies
# d -- downloads of the dependencies from files.kde.org
# b -- build directory for the dependencies
# krita_build -- build directory for krita itself
# krita.appdir -- install directory for krita and the dependencies
#
# Krita should have been checked out in the build deps phase
#
# Get Krita
if [ ! -d $BUILD_PREFIX/krita ] ; then
  git clone  --depth 1 git://anongit.kde.org/krita $BUILD_PREFIX/krita
fi

cd $BUILD_PREFIX/krita/
git_pull_rebase_helper
cd -

rm -rf $BUILD_PREFIX/krita_build || true
mkdir -p $BUILD_PREFIX/krita_build
cd $BUILD_PREFIX/krita_build

cmake $BUILD_PREFIX/krita \
    -DCMAKE_INSTALL_PREFIX:PATH=$BUILD_PREFIX/krita.appdir/usr \
    -DDEFINE_NO_DEPRECATED=1 \
    -DCMAKE_BUILD_TYPE=Release \
    -DFOUNDATION_BUILD=1 \
    -DHIDE_SAFE_ASSERTS=ON \
    -DBUILD_TESTING=FALSE \
    -DPYQT_SIP_DIR_OVERRIDE=$QTDIR/share/sip/ \
    -DHAVE_MEMORY_LEAK_TRACKER=FALSE
    
# build
make -j10 install

