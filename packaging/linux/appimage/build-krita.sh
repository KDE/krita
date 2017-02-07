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

# if the library path doesn't point to our usr/lib, linking will be broken and we won't find all deps either
export LD_LIBRARY_PATH=/usr/lib64/:/usr/lib:/krita.appdir/usr/lib

git_pull_rebase_helper()
{
	git reset --hard HEAD
        git pull
}

# Use the new compiler
. /opt/rh/devtoolset-3/enable


# Workaround for: On CentOS 6, .pc files in /usr/lib/pkgconfig are not recognized
# However, this is where .pc files get installed when bulding libraries... (FIXME)
# I found this by comparing the output of librevenge's "make install" command
# between Ubuntu and CentOS 6
ln -sf /usr/share/pkgconfig /usr/lib/pkgconfig


# A krita build layout looks like this:
# krita/ -- the source directory
# krita/3rdparty -- the cmake3 definitions for the dependencies
# d -- downloads of the dependencies from files.kde.org
# b -- build directory for the dependencies
# krita_build -- build directory for krita itself
# krita.appdir -- install directory for krita and the dependencies

# Get Krita
if [ ! -d /krita ] ; then
	git clone  --depth 1 https://github.com/KDE/krita.git /krita
fi

cd /krita/
git_pull_rebase_helper

cd /

# If the environment variable DO_NOT_BUILD_KRITA is set to something,
# then stop here. This is for docker hub which has a timeout that
# prevents us from building in one go.
# if [ ! -z "$DO_NOT_BUILD_KRITA" ] ; then
#  exit 0
# fi

mkdir -p /krita_build
cd /krita_build
cmake3 ../krita \
    -DCMAKE_INSTALL_PREFIX:PATH=/krita.appdir/usr \
    -DDEFINE_NO_DEPRECATED=1 \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBUILD_TESTING=FALSE \
    -DKDE4_BUILD_TESTS=FALSE \
    -DHAVE_MEMORY_LEAK_TRACKER=FALSE
    
# build
make -j4 

