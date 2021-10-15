#!/bin/bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# Halt on errors and be verbose about what we are doing
set -e
set -x

# Read in our parameters
export BUILD_PREFIX=$1
export KRITA_SOURCES=$2
export CHANNEL="${3}"

# Save some frequently referenced locations in variables for ease of use / updating
export APPDIR=$BUILD_PREFIX/krita.appdir
export PLUGINS=$APPDIR/usr/lib/kritaplugins/

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment.
# That's not always the case, so make sure it is
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# We want to use $prefix/deps/usr/ for all our dependencies
export DEPS_INSTALL_PREFIX=$BUILD_PREFIX/deps/usr
export DOWNLOADS_DIR=$BUILD_PREFIX/downloads/

# Setup variables needed to help everything find what we built
ARCH=`dpkg --print-architecture`
TRIPLET=`gcc -dumpmachine`
export LD_LIBRARY_PATH=$DEPS_INSTALL_PREFIX/lib/:$DEPS_INSTALL_PREFIX/lib/$TRIPLET/:$APPDIR/usr/lib/:$LD_LIBRARY_PATH
export PATH=$DEPS_INSTALL_PREFIX/bin/:$PATH
export PKG_CONFIG_PATH=$DEPS_INSTALL_PREFIX/share/pkgconfig/:$DEPS_INSTALL_PREFIX/lib/pkgconfig/:/usr/lib/pkgconfig/:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$DEPS_INSTALL_PREFIX:$CMAKE_PREFIX_PATH
# https://docs.python.org/3.8/using/cmdline.html#envvar-PYTHONHOME
if [ -d $DEPS_INSTALL_PREFIX/sip ] ; then
export PYTHONPATH=$DEPS_INSTALL_PREFIX/sip
fi
export PYTHONHOME=$DEPS_INSTALL_PREFIX

if [ -n "${CHANNEL}" ]; then
    # download
    # XXX: bundle this inside the Docker image *and* make it portable to ARM
    mkdir -p $DOWNLOADS_DIR
    cd $DOWNLOADS_DIR
    wget "https://files.kde.org/krita/build/AppImageUpdate-x86_64.AppImage" -O AppImageUpdate
    echo -n "ebc4763e8eac6aa7b9dfcbea77ec07d2e01fa1b9f10a38d4af0fc040bc965c1f AppImageUpdate" | sha256sum -c -
fi

# Switch over to our build prefix
cd $BUILD_PREFIX

#
# Now we can get the process started!
#

# Step 0: place the translations where ki18n and Qt look for them
if [ -d $APPDIR/usr/share/locale ] ; then
    rsync -prul $APPDIR/usr/share/locale $APPDIR/usr/share/krita
    rm -rf $APPDIR/usr/share/locale
fi

# Step 1: Copy over all the resources provided by dependencies that we need
cp -r $DEPS_INSTALL_PREFIX/share/locale $APPDIR/usr/share/krita
cp -r $DEPS_INSTALL_PREFIX/share/kf5 $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/share/mime $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/lib/python3.8 $APPDIR/usr/lib
if [ -d $DEPS_INSTALL_PREFIX/share/sip ] ; then
cp -r $DEPS_INSTALL_PREFIX/share/sip $APPDIR/usr/share
fi
cp -r $DEPS_INSTALL_PREFIX/translations $APPDIR/usr/

if [ -d $APPDIR/usr/lib/python3.8/site-packages ]; then
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/packaging*
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/pip*
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/pyparsing*
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/PyQt_builder*
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/setuptools*
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/sip*
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/toml*
    rm -rf $APPDIR/usr/lib/python3.8/site-packages/easy-install.pth
fi

# Step 2: Relocate binaries from the architecture specific directory as required for Appimages
if [[ -d "$APPDIR/usr/lib/$TRIPLET" ]] ; then
  rsync -prul $APPDIR/usr/lib/$TRIPLET/ $APPDIR/usr/lib/
  rm -rf $APPDIR/usr/lib/$TRIPLET/
fi

# Step 3: Update the rpath in the various plugins we have to make sure they'll be loadable in an Appimage context
for lib in $PLUGINS/*.so*; do
  patchelf --set-rpath '$ORIGIN/..' $lib;
done

for lib in $APPDIR/usr/lib/python3.8/site-packages/PyQt5/*.so*; do
  patchelf --set-rpath '$ORIGIN/../..' $lib;
done

for lib in $APPDIR/usr/lib/python3.8/lib-dynload/*.so*; do
  patchelf --set-rpath '$ORIGIN/../..' $lib;
done

patchelf --set-rpath '$ORIGIN/../../../..' $APPDIR/usr/lib/qml/org/krita/draganddrop/libdraganddropplugin.so
patchelf --set-rpath '$ORIGIN/../../../..' $APPDIR/usr/lib/qml/org/krita/sketch/libkritasketchplugin.so
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so
if [ -f $APPDIR/usr/lib/python3.8/site-packages/PyQt5/sip.so ] ; then
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/python3.8/site-packages/PyQt5/sip.so
fi


# Step 4: Install AppImageUpdate
if [ -f $DOWNLOADS_DIR/AppImageUpdate ]; then
       cp $DOWNLOADS_DIR/AppImageUpdate $APPDIR/usr/bin/
       chmod +x $APPDIR/usr/bin/AppImageUpdate
fi


# Step 5: Find out what version of Krita we built and give the Appimage a proper name
cd $BUILD_PREFIX/krita-build

KRITA_VERSION=$(grep "#define KRITA_VERSION_STRING" libs/version/kritaversion.h | cut -d '"' -f 2)
# Also find out the revision of Git we built
# Then use that to generate a combined name we'll distribute
cd $KRITA_SOURCES
if git rev-parse --is-inside-work-tree; then
	GIT_REVISION=$(git rev-parse --short HEAD)
	export VERSION=$KRITA_VERSION-$GIT_REVISION
	VERSION_TYPE="development"

	if [ -z "${CHANNEL}" ]; then
        BRANCH="$(git rev-parse --abbrev-ref HEAD)"
        if [ "$BRANCH" = "master" ]; then
            CHANNEL="Next"
        elif [[ "${BRANCH}" =~ krita/.* ]]; then
            CHANNEL="Plus"
        fi
    fi
else
	export VERSION=$KRITA_VERSION

    pushd $BUILD_PREFIX/krita-build

    #if KRITA_BETA is set, set channel to Beta, otherwise set it to stable
    is_beta=0
    grep "define KRITA_BETA 1" libs/version/kritaversion.h || is_beta=$?

    is_rc=0
    grep "define KRITA_RC 1" libs/version/kritaversion.h || is_rc=$?

    popd

    if [ $is_beta -eq 0 ]; then
        VERSION_TYPE="development"
    else
        VERSION_TYPE="stable"
    fi

    if [ -z "${CHANNEL}" ]; then
        if [ $is_beta -eq 0 ] || [ $is_rc -eq 0 ]; then
            CHANNEL="Beta"
        else
            CHANNEL="Stable"
        fi
    fi
fi

DATE=$(git log -1 --format="%ct" | xargs -I{} date -d @{} +%Y-%m-%d)
if [ "$DATE" = "" ] ; then
	DATE=$(date +%Y-%m-%d)
fi

sed -e "s|<release version=\"\" date=\"\" />|<release version=\"$VERSION\" date=\"$DATE\" type=\"$VERSION_TYPE\"/>|" -i $APPDIR/usr/share/metainfo/org.kde.krita.appdata.xml

if [ -n "${CHANNEL}" ]; then # if channel argument is set, we wish to embed update information
    # set zsync url for linuxdeployqt
    if [ "$CHANNEL" = "Next" ]; then
        ZSYNC_URL="zsync|https://binary-factory.kde.org/job/Krita_Nightly_Appimage_Build/lastSuccessfulBuild/artifact/Krita-${CHANNEL}-x86_64.appimage.zsync"
    elif [ "$CHANNEL" = "Plus" ]; then
        ZSYNC_URL="zsync|https://binary-factory.kde.org/job/Krita_Stable_Appimage_Build/lastSuccessfulBuild/artifact/Krita-${CHANNEL}-x86_64.appimage.zsync"
    elif [ "$CHANNEL" = "Stable" ]; then
        ZSYNC_URL="zsync|https://download.kde.org/stable/krita/updates/Krita-${CHANNEL}-x86_64.appimage.zsync"
    elif [ "$CHANNEL" = "Beta" ]; then
        ZSYNC_URL="zsync|https://download.kde.org/unstable/krita/updates/Krita-${CHANNEL}-x86_64.appimage.zsync"
    fi
fi

# Return to our build root
cd $BUILD_PREFIX

# place the icon where linuxdeployqt seems to expect it
find $APPDIR -name krita.png
cp $APPDIR/usr/share/icons/hicolor/256x256/apps/krita.png $APPDIR
ls $APPDIR

if [ -n "$STRIP_APPIMAGE" ]; then
    # strip debugging information
    function find-elf-files {
        # python libraries are not strippable (strip fails with error)
        find $1 -type f -name "*" -not -name "*.o" -not -path "*/python3.8/*" -exec sh -c '
            case "$(head -n 1 "$1")" in
            ?ELF*) exit 0;;
            esac
            exit 1
            ' sh {} \; -print
    }

    TEMPFILE=`tempfile`
    find-elf-files $APPDIR > $TEMPFILE

    for i in `cat $TEMPFILE`; do
        strip -v --strip-unneeded --strip-debug $i
    done

    rm -f $TEMPFILE
fi

# GStreamer + QTMultimedia Support

export GSTREAMER_TARGET=$APPDIR/usr/lib/gstreamer-1.0

# First, lets get the GSTREAMER plugins installed.
# For now, I'm just going to install all plugins. Once it's working, I'll start picking individual libs that Krita actually needs.
mkdir -p $GSTREAMER_TARGET
install -Dm 755 /usr/lib/$TRIPLET/gstreamer-1.0/*.so $GSTREAMER_TARGET/
install -Dm 755 /usr/lib/$TRIPLET/gstreamer1.0/gstreamer-1.0/gst-plugin-scanner $GSTREAMER_TARGET/gst-plugin-scanner
install -Dm 755 /usr/lib/$TRIPLET/libgstreamer-1.0.so $APPDIR/usr/lib/

GSTREAMER_BINARIES="-executable=${GSTREAMER_TARGET}/gst-plugin-scanner -executable=${APPDIR}/usr/lib/libgstreamer-1.0.so"
for plugin in alsa app audioconvert audioparsers audioresample autodetect \
              coreelements id3demux jack mpg123 mulaw playback pulse \
              typefindfunctions wavparse; do
	GSTREAMER_BINARIES="${GSTREAMER_BINARIES} -executable=${GSTREAMER_TARGET}/libgst${plugin}.so"
done

# Second, we need the mediaservice QT plugins to be installed.

mkdir -p $APPDIR/usr/plugins/mediaservice
install -Dm 755 $DEPS_INSTALL_PREFIX/plugins/mediaservice/*.so $APPDIR/usr/plugins/mediaservice/
QT_MEDIA_SERVICES=""
for plugin in audiodecoder mediaplayer mediacapture; do
     QT_MEDIA_SERVICES="${QT_MEDIA_SERVICES} -executable=${APPDIR}/usr/plugins/mediaservice/libgst${plugin}.so"
done

# Step 4: Build the image!!!
linuxdeployqt $APPDIR/usr/share/applications/org.kde.krita.desktop \
  -executable=$APPDIR/usr/bin/krita \
  $GSTREAMER_BINARIES \
  $QT_MEDIA_SERVICES \
  -qmldir=$DEPS_INSTALL_PREFIX/qml \
  -verbose=2 \
  -bundle-non-qt-libs \
  -extra-plugins=mediaservice,$PLUGINS,$APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so,$APPDIR/usr/lib//qml/org/krita/sketch/libkritasketchplugin.so,$APPDIR/usr/lib/qml/org/krita/draganddrop/libdraganddropplugin.so  \
  -updateinformation="${ZSYNC_URL}" \
  -appimage

# Generate a new name for the Appimage file and rename it accordingly

if [[ $ARCH == "arm64" ]]; then
  APPIMAGE_ARCHITECTURE="aarch64"
elif [[ $ARCH == "amd64" ]]; then
  APPIMAGE_ARCHITECTURE="x86_64"
else
  APPIMAGE_ARCHITECTURE=$ARCH
fi

OLD_APPIMAGE_NAME="Krita-${VERSION}-${APPIMAGE_ARCHITECTURE}.AppImage"
NEW_APPIMAGE_NAME="krita-${VERSION}-${APPIMAGE_ARCHITECTURE}.appimage"
mv ${OLD_APPIMAGE_NAME} ${NEW_APPIMAGE_NAME}
