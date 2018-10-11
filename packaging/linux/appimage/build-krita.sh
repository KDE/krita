#!/bin/bash

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
export LD_LIBRARY_PATH=$DEPS_INSTALL_PREFIX/lib:$LD_LIBRARY_PATH
export PATH=$DEPS_INSTALL_PREFIX/bin:$PATH
export PKG_CONFIG_PATH=$DEPS_INSTALL_PREFIX/share/pkgconfig:$DEPS_INSTALL_PREFIX/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$DEPS_INSTALL_PREFIX:$CMAKE_PREFIX_PATH
export PYTHONPATH=$DEPS_INSTALL_PREFIX/sip:$DEPS_INSTALL_PREFIX/lib/python3.5/site-packages:$DEPS_INSTALL_PREFIX/lib/python3.5
export PYTHONHOME=$DEPS_INSTALL_PREFIX

# Make sure our build directory exists
if [ ! -d $BUILD_PREFIX/krita-build/ ] ; then
    mkdir -p $BUILD_PREFIX/krita-build/
fi

# Now switch to it
cd $BUILD_PREFIX/krita-build/

# Determine how many CPUs we have
CPU_COUNT=`grep processor /proc/cpuinfo | wc -l`

# Configure Krita
cmake $KRITA_SOURCES \
    -DCMAKE_INSTALL_PREFIX:PATH=$BUILD_PREFIX/krita.appdir/usr \
    -DDEFINE_NO_DEPRECATED=1 \
    -DCMAKE_BUILD_TYPE=Release \
    -DFOUNDATION_BUILD=1 \
    -DHIDE_SAFE_ASSERTS=OFF \
    -DBUILD_TESTING=FALSE \
    -DUSE_QT_XCB=ON \
    -DPYQT_SIP_DIR_OVERRIDE=$DEPS_INSTALL_PREFIX/share/sip/ \
    -DHAVE_MEMORY_LEAK_TRACKER=FALSE
    
# Build and Install Krita (ready for the next phase)
make -j$CPU_COUNT install

