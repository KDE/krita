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
# https://docs.python.org/3.10/using/cmdline.html#envvar-PYTHONHOME
if [ -d $DEPS_INSTALL_PREFIX/sip ] ; then
export PYTHONPATH=$DEPS_INSTALL_PREFIX/sip
fi
export PYTHONHOME=$DEPS_INSTALL_PREFIX

source ${KRITA_SOURCES}/packaging/linux/appimage/override_compiler.sh.inc

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

# Step 1: Copy over all necessary resources required by dependencies or libraries that are missed by linuxdeployqt
cp -r $DEPS_INSTALL_PREFIX/share/locale $APPDIR/usr/share/krita
cp -r $DEPS_INSTALL_PREFIX/share/kf5 $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/share/mime $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/lib/python3.10 $APPDIR/usr/lib
if [ -d $DEPS_INSTALL_PREFIX/share/sip ] ; then
cp -r $DEPS_INSTALL_PREFIX/share/sip $APPDIR/usr/share
fi

cp -r $DEPS_INSTALL_PREFIX/translations $APPDIR/usr/

if [ ! -d $APPDIR/usr/libstdcpp-fallback/ ] ; then
    mkdir -p $APPDIR/usr/libstdcpp-fallback/
    cd $APPDIR/usr/libstdcpp-fallback/
    cp  /usr/lib/x86_64-linux-gnu/libstdc++.so.6.* ./
    ln -s `find ./ -name 'libstdc++.so.6.*' -maxdepth 1 -type f | head -n1 | xargs basename` libstdc++.so.6
fi

mkdir $BUILD_PREFIX/krita-apprun-build
(
    cd $BUILD_PREFIX/krita-apprun-build
    cmake -DCMAKE_BUILD_TYPE=Release $KRITA_SOURCES/packaging/linux/appimage/krita-apprun/
    cmake --build .
    cp AppRun $APPDIR
)
rm -rf $BUILD_PREFIX/krita-apprun-build

if [ -d $APPDIR/usr/lib/python3.10/site-packages ]; then
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/packaging*
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/pip*
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/pyparsing*
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/PyQt_builder*
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/setuptools*
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/sip*
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/toml*
    rm -rf $APPDIR/usr/lib/python3.10/site-packages/easy-install.pth
fi

## == MLT Dependencies and Resources ==
cp -r $DEPS_INSTALL_PREFIX/share/mlt-7 $APPDIR/usr/share/mlt-7
cp -r $DEPS_INSTALL_PREFIX/lib/mlt-7 $APPDIR/usr/lib/mlt
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libmlt*.so* $APPDIR/usr/lib/
cp -av $DEPS_INSTALL_PREFIX/bin/melt $APPDIR/usr/bin/melt

export MLT_BINARIES=""
for BIN in $APPDIR/usr/lib/libmlt*.so*; do
  MLT_BINARIES="${MLT_BINARIES} -executable=${BIN}"
done

for BIN in $APPDIR/usr/lib/mlt/*.so*; do
  MLT_BINARIES="${MLT_BINARIES} -executable=${BIN}"
done

for BIN in $APPDIR/usr/bin/melt; do
  MLT_BINARIES="${MLT_BINARIES} -executable=${BIN}"
done


## == FFMPEG Dependencies and Resources ==
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libav*.s* $APPDIR/usr/lib/
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libpostproc*.s* $APPDIR/usr/lib/
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libsw*.s* $APPDIR/usr/lib/
cp -av $DEPS_INSTALL_PREFIX/bin/ff* $APPDIR/usr/bin/

export FFMPEG_BINARIES=""

for BIN in $APPDIR/usr/bin/ff*; do
  FFMPEG_BINARIES="${FFMPEG_BINARIES} -executable=${BIN}"
done

for BIN in $APPDIR/usr/lib/libav*.s*; do
  FFMPEG_BINARIES="${FFMPEG_BINARIES} -executable=${BIN}"
done;

for BIN in $APPDIR/usr/lib/libpostproc*; do
  FFMPEG_BINARIES="${FFMPEG_BINARIES} -executable=${BIN}"
done;

for BIN in $APPDIR/usr/lib/libsw*.s*; do
  FFMPEG_BINARIES="${FFMPEG_BINARIES} -executable=${BIN}"
done;

# Step 2: Relocate binaries from the architecture specific directory as required for Appimages
if [[ -d "$APPDIR/usr/lib/$TRIPLET" ]] ; then
  rsync -prul $APPDIR/usr/lib/$TRIPLET/ $APPDIR/usr/lib/
  rm -rf $APPDIR/usr/lib/$TRIPLET/
fi

# Step 3: Update the rpath in the various plugins we have to make sure they'll be loadable in an Appimage context
for lib in $PLUGINS/*.so*; do
  patchelf --set-rpath '$ORIGIN/..' $lib;
done

for lib in $APPDIR/usr/lib/python3.10/site-packages/PyQt5/*.so*; do
  patchelf --set-rpath '$ORIGIN/../..' $lib;
done

for lib in $APPDIR/usr/lib/python3.10/lib-dynload/*.so*; do
  patchelf --set-rpath '$ORIGIN/../..' $lib;
done

patchelf --set-rpath '$ORIGIN/../../../..' $APPDIR/usr/lib/qml/org/krita/draganddrop/libdraganddropplugin.so
patchelf --set-rpath '$ORIGIN/../../../..' $APPDIR/usr/lib/qml/org/krita/sketch/libkritasketchplugin.so
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so
if [ -f $APPDIR/usr/lib/python3.10/site-packages/PyQt5/sip.so ] ; then
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/python3.10/site-packages/PyQt5/sip.so
fi

# Step 4: Find out what version of Krita we built and give the Appimage a proper name
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

        if [ "${JOB_NAME}" == "Krita_Nightly_Appimage_Build" ]; then
            CHANNEL="Next"
        elif [ "${JOB_NAME}" == "Krita_Stable_Appimage_Build" ]; then
            CHANNEL="Plus"
        elif [ "$BRANCH" = "master" ]; then
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

# Step 5: Install AppImageUpdate

if [[ $ARCH == "amd64" ]]; then
    # download
    # XXX: bundle this inside the Docker image *and* make it portable to ARM
    mkdir -p $DOWNLOADS_DIR
    cd $DOWNLOADS_DIR

    UPDATER_SHA256=414f10d9ab2dc72dc6874dbdb99454227124473a7ae691db2288d60f14f810fe

    if ! test -f "AppImageUpdate" || ! echo -n "$UPDATER_SHA256 AppImageUpdate" | sha256sum -c -; then
        wget "https://files.kde.org/krita/build/AppImageUpdate-x86_64.AppImage" -O AppImageUpdate
        echo -n "$UPDATER_SHA256 AppImageUpdate" | sha256sum -c -
    fi

    cd $BUILD_PREFIX
fi

if [ -f $DOWNLOADS_DIR/AppImageUpdate ]; then
       cp $DOWNLOADS_DIR/AppImageUpdate $APPDIR/usr/bin/
       chmod +x $APPDIR/usr/bin/AppImageUpdate
fi

# place the icon where linuxdeployqt seems to expect it
find $APPDIR -name krita.png
cp $APPDIR/usr/share/icons/hicolor/256x256/apps/krita.png $APPDIR
ls $APPDIR

if [ -n "$STRIP_APPIMAGE" ]; then
    # strip debugging information
    function find-elf-files {
        # * python libraries are not strippable (strip fails with error)
        # * AppImage packages should not be stripped, because it breaks them
        find $1 -type f -name "*" -not -name "*.o" -not -path "*/python3.10/*" -not -name "AppImageUpdate" -not -name "libstdc++.so.6.*" -exec sh -c '
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

# Step 4: Build the image!!!
linuxdeployqt $APPDIR/usr/share/applications/org.kde.krita.desktop \
  -executable=$APPDIR/usr/bin/krita \
  ${MLT_BINARIES} \
  ${FFMPEG_BINARIES} \
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
