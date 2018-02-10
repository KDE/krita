  #!/bin/bash
#
# Build all Krita's dependencies on Ubuntu 14.04.
#
# Prerequisites: cmake git build-essential libxcb-keysyms1-dev plus all deps for Qt5
#


# Halt on errors
set -e

# Be verbose
set -x

export BUILD_PREFIX=/media/krita/_devel
export QTDIR=$BUILD_PREFIX/deps/usr
export LD_LIBRARY_PATH=$QTDIR/lib:$BUILD_PREFIX/i/lib:$LD_LIBRARY_PATH
export PATH=$QTDIR/bin:$BUILD_PREFIX/i/bin:$PATH
export PKG_CONFIG_PATH=$QTDIR/share/pkgconfig:$QTDIR/lib/pkgconfig:$BUILD_PREFIX/i/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$BUILD_PREFIX:$QTDIR:$CMAKE_PREFIX_PATH

#
# Add the deps we can install from Ubuntu 14.04
#
install_deps()
{
  apt update
  apt upgrade
  apt-get install build-essential \
  cmake3 \
  git \
  libcairo-perl \
  libgl1-mesa-dev \
  libglib-perl \
  libgtk2-perl \
  libx11-dev \
  libxcb1-dev \
  libxcb-glx0-dev \
  libxcb-keysyms1-dev \
  libxext-dev \
  libxfixes-dev \
  libxi-dev \
  libxrender-dev \
  mesa-common-dev \
  libxcb-util0-dev \
  libgsl0-dev 
}

install_deps || true

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

git_pull_rebase_helper()
{
  git reset --hard HEAD
  git pull
}

# A krita build layout looks like this:
# krita/ -- the source directory
# krita/3rdparty -- the cmake definitions for the dependencies
# d -- downloads of the dependencies from files.kde.org
# b -- build directory for the dependencies
# deps -- the location for the built dependencies
# build -- build directory for krita itself
# krita.appdir -- install directory for krita and the dependencies

# Get Krita
if [ ! -d $BUILD_PREFIX/krita ] ; then
  git clone  --depth 1 git://anongit.kde.org/krita $BUILD_PREFIX/krita
fi

cd $BUILD_PREFIX/krita
git_pull_rebase_helper
cd -

# Create the build dir for the 3rdparty deps
if [ ! -d $BUILD_PREFIX/b ] ; then
    mkdir $BUILD_PREFIX/b
fi    

if [ ! -d $BUILD_PREFIX/d ] ; then
    mkdir $BUILD_PREFIX/d
fi

if [ ! -d $BUILD_PREFIX/i ] ; then
    mkdir $BUILD_PREFIX/i
fi

if [ ! -d $QTDIR ] ; then
    mkdir -p $QTDIR
fi

cd $BUILD_PREFIX/b
#rm -rf * || true

cmake $BUILD_PREFIX/krita/3rdparty \
    -DCMAKE_INSTALL_PREFIX:PATH=$QTDIR \
    -DINSTALL_ROOT=$QTDIR \
    -DEXTERNALS_DOWNLOAD_DIR=$BUILD_PREFIX/d
    
cmake --build . --config RelWithDebInfo --target ext_png
cmake --build . --config RelWithDebInfo --target ext_tiff
cmake --build . --config RelWithDebInfo --target ext_jpeg
cmake --build . --config RelWithDebInfo --target ext_boost
cmake --build . --config RelWithDebInfo --target ext_eigen3
cmake --build . --config RelWithDebInfo --target ext_exiv2
cmake --build . --config RelWithDebInfo --target ext_fftw3
cmake --build . --config RelWithDebInfo --target ext_lcms2
cmake --build . --config RelWithDebInfo --target ext_ocio
cmake --build . --config RelWithDebInfo --target ext_openexr
cmake --build . --config RelWithDebInfo --target ext_vc
cmake --build . --config RelWithDebInfo --target ext_libraw
#cmake --build . --config RelWithDebInfo --target ext_gsl
cmake --build . --config RelWithDebInfo --target ext_python
cmake --build . --config RelWithDebInfo --target ext_qt
cmake --build . --config RelWithDebInfo --target ext_freetype
cmake --build . --config RelWithDebInfo --target ext_fontconfig
cmake --build . --config RelWithDebInfo --target ext_poppler
cmake --build . --config RelWithDebInfo --target ext_kcrash
cmake --build . --config RelWithDebInfo --target ext_gmic
cmake --build . --config RelWithDebInfo --target ext_sip
cmake --build . --config RelWithDebInfo --target ext_pyqt


