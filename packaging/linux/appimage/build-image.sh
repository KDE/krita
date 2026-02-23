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

# Save some frequently referenced locations in variables for ease of use / updating
export APPDIR=${KRITA_APPDIR_PATH:-$BUILD_PREFIX/krita.appdir}
export PLUGINS=$APPDIR/usr/lib/kritaplugins/

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment.
# That's not always the case, so make sure it is
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# We want to use $prefix/deps/usr/ for all our dependencies
export DEPS_INSTALL_PREFIX=${KRITA_DEPS_PATH:-$BUILD_PREFIX/deps/usr}
export BUILD_DIR=${KRITA_BUILD_PATH:-$BUILD_PREFIX/krita-build/}
export DOWNLOADS_DIR=${KRITA_DOWNLOADS_PATH:-$BUILD_PREFIX/downloads/}

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

# Detect Python version; for example 'python3.13'
export PYTHON_VER=`find $DEPS_INSTALL_PREFIX/lib -maxdepth 1 -name 'python*' -type d`
export PYTHON_VER=`basename $PYTHON_VER`
if [ -d $DEPS_INSTALL_PREFIX/lib/$PYTHON_VER/site-packages/PyQt5/ ]; then
  export PYQT_VER=PyQt5
else
  export PYQT_VER=PyQt6
fi

# add our own linuxdeployqt to our PATH environment if available
if [ -d $DEPS_INSTALL_PREFIX/appimage-tools/bin ]; then
  export PATH=$DEPS_INSTALL_PREFIX/appimage-tools/bin/:$PATH
fi

source ${KRITA_SOURCES}/packaging/linux/appimage/override_compiler.sh.inc

if [[ $ARCH == "arm64" ]]; then
  APPIMAGE_ARCHITECTURE="aarch64"
elif [[ $ARCH == "amd64" ]]; then
  APPIMAGE_ARCHITECTURE="x86_64"
else
  APPIMAGE_ARCHITECTURE=$ARCH
fi

# Step 0: Find out what version of Krita we built and give the Appimage a proper name
cd $BUILD_DIR

KRITA_VERSION=$(grep "#define KRITA_VERSION_STRING" libs/version/kritaversion.h | cut -d '"' -f 2)
# Also find out the revision of Git we built
# Then use that to generate a combined name we'll distribute
cd $KRITA_SOURCES

if [[ "${CI_COMMIT_BRANCH}" =~ release/.* ]]; then
    BRANCH_VERSION=${CI_COMMIT_BRANCH/#release\//}
    echo "Found a release branch: ${CI_COMMIT_BRANCH} using version: ${BRANCH_VERSION}"

    if [[ "$BRANCH_VERSION" != "$KRITA_VERSION" ]]; then
        echo "WARNING: Branch version does not coincide with the version in kritaversion.h:"
        echo "    branch version: $BRANCH_VERSION"
        echo "    kritaversion.h: $KRITA_VERSION"
    fi

    if [[ "${KRITA_VERSION}" =~ -beta$ ]]; then
        echo "ERROR: beta version does not have a numeric suffix, please change it to \"beta1\""
        exit 3
    fi

    if [[ "${KRITA_VERSION}" =~ -rc$ ]]; then
        echo "ERROR: release candidate version does not have a numeric suffix, please change it to \"rc1\""
        exit 4
    fi

    export VERSION=$KRITA_VERSION
    NEW_APPIMAGE_NAME="krita-${VERSION}-${APPIMAGE_ARCHITECTURE}.AppImage"

    pushd $BUILD_DIR

    #if KRITA_BETA is set, set channel to Beta, otherwise set it to stable
    is_beta=0
    grep "define KRITA_BETA 1" libs/version/kritaversion.h || is_beta=$?

    is_rc=0
    grep "define KRITA_RC 1" libs/version/kritaversion.h || is_rc=$?

    popd

    if [ $is_beta -eq 0 ] || [ $is_rc -eq 0 ]; then
        CHANNEL="Beta"
        ZSYNC_URL="zsync|https://download.kde.org/unstable/krita/updates/Krita-${CHANNEL}-${APPIMAGE_ARCHITECTURE}.appimage.zsync"
        ZSYNC_SOURCE_URL="https://download.kde.org/unstable/krita/${KRITA_VERSION}/${NEW_APPIMAGE_NAME}"
        VERSION_TYPE="development"
    else
        CHANNEL="Stable"
        ZSYNC_URL="zsync|https://download.kde.org/stable/krita/updates/Krita-${CHANNEL}-${APPIMAGE_ARCHITECTURE}.appimage.zsync"
        ZSYNC_SOURCE_URL="https://download.kde.org/stable/krita/${KRITA_VERSION}/${NEW_APPIMAGE_NAME}"
        VERSION_TYPE="stable"
    fi

else
    VERSION_TYPE="development"

    if git rev-parse --is-inside-work-tree; then
        GIT_REVISION=$(git rev-parse --short HEAD)
        export VERSION=$KRITA_VERSION-$GIT_REVISION
    else
        export VERSION=$KRITA_VERSION
    fi

    NEW_APPIMAGE_NAME="krita-${VERSION}-${APPIMAGE_ARCHITECTURE}.AppImage"

    if [[ "${CI_COMMIT_BRANCH}" =~ krita/.* ]]; then
        ESCAPED_COMMIT_BRANCH="${CI_COMMIT_BRANCH//\//-}"

        CHANNEL="Plus"
        ZSYNC_URL="zsync|https://autoconfig.kde.org/krita/updates/plus/linux/Krita-Plus-${APPIMAGE_ARCHITECTURE}.appimage.zsync"
        ZSYNC_SOURCE_URL="https://autoconfig.kde.org/krita/updates/plus/linux/${NEW_APPIMAGE_NAME}"
    else
        CHANNEL="Next"
        ZSYNC_URL="zsync|https://autoconfig.kde.org/krita/updates/next/linux/Krita-Next-${APPIMAGE_ARCHITECTURE}.appimage.zsync"
        ZSYNC_SOURCE_URL="https://autoconfig.kde.org/krita/updates/next/linux/${NEW_APPIMAGE_NAME}"
    fi
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

# Step 1: Copy over all necessary resources required by dependencies or libraries that are missed by linuxdeployqt
cp -r $DEPS_INSTALL_PREFIX/share/locale $APPDIR/usr/share/krita
if [ -d $DEPS_INSTALL_PREFIX/share/kf5 ]; then
    cp -r $DEPS_INSTALL_PREFIX/share/kf5 $APPDIR/usr/share
else
    cp -r $DEPS_INSTALL_PREFIX/share/kf6 $APPDIR/usr/share
fi
cp -r $DEPS_INSTALL_PREFIX/share/mime $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/lib/$PYTHON_VER $APPDIR/usr/lib
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

if [ -d $APPDIR/usr/lib/$PYTHON_VER/site-packages ]; then
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/packaging*
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/pip*
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/pyparsing*
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/PyQt_builder*
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/setuptools*
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/sip*
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/toml*
    rm -rf $APPDIR/usr/lib/$PYTHON_VER/site-packages/easy-install.pth
fi

## Font related deps are explicitly ignored by AppImage build script,
## so we should copy them manually
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libfontconfig.so.1* $APPDIR/usr/lib/
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libharfbuzz.so.0* $APPDIR/usr/lib/
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libfribidi.so.0* $APPDIR/usr/lib/
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libfreetype.so.6* $APPDIR/usr/lib/

## == MLT Dependencies and Resources ==
cp -r $DEPS_INSTALL_PREFIX/share/mlt-7 $APPDIR/usr/share/mlt-7
cp -r $DEPS_INSTALL_PREFIX/lib/mlt-7 $APPDIR/usr/lib/mlt-7
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libmlt*.so* $APPDIR/usr/lib/

export MLT_BINARIES=""
for BIN in $APPDIR/usr/lib/libmlt*.so*; do
  MLT_BINARIES="${MLT_BINARIES} -executable=${BIN}"
done

for BIN in $APPDIR/usr/lib/mlt-7/*.so*; do
  MLT_BINARIES="${MLT_BINARIES} -executable=${BIN}"
done

## == FFMPEG Dependencies and Resources ==
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libav*.s* $APPDIR/usr/lib/
## TODO: remove (libpostproc is disabled in our builds of ffmpeg)
##cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libpostproc*.s* $APPDIR/usr/lib/
cp -av --preserve=links $DEPS_INSTALL_PREFIX/lib/libsw*.s* $APPDIR/usr/lib/
cp -av $DEPS_INSTALL_PREFIX/bin/ff* $APPDIR/usr/bin/

export FFMPEG_BINARIES=""

for BIN in $APPDIR/usr/bin/ff*; do
  FFMPEG_BINARIES="${FFMPEG_BINARIES} -executable=${BIN}"
done

for BIN in $APPDIR/usr/lib/libav*.s*; do
  FFMPEG_BINARIES="${FFMPEG_BINARIES} -executable=${BIN}"
done;

## TODO: remove (libpostproc is disabled in our builds of ffmpeg)
#for BIN in $APPDIR/usr/lib/libpostproc*; do
#  FFMPEG_BINARIES="${FFMPEG_BINARIES} -executable=${BIN}"
#done;

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

if [ -d $APPDIR/usr/lib/$PYTHON_VER/site-packages/$PYQT_VER/ ] ; then
  for lib in $APPDIR/usr/lib/$PYTHON_VER/site-packages/$PYQT_VER/*.so*; do
    patchelf --set-rpath '$ORIGIN/../..' $lib;
  done
fi

for lib in $APPDIR/usr/lib/$PYTHON_VER/lib-dynload/*.so*; do
  patchelf --set-rpath '$ORIGIN/../..' $lib;
done

if [[ -n $KRITACI_ALLOW_NO_PYQT && ! -f $APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so ]]; then
  echo "WARNING: not found $APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so, skipping..."
else
  patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so
fi

if [ -f $APPDIR/usr/lib/$PYTHON_VER/site-packages/$PYQT_VER/sip.so ] ; then
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/$PYTHON_VER/site-packages/$PYQT_VER/sip.so
fi

# Step 4: Update version and update information

cd $KRITA_SOURCES

DATE=$(git log -1 --format="%ct" | xargs -I{} date -d @{} +%Y-%m-%d)
if [ "$DATE" = "" ] ; then
        DATE=$(date +%Y-%m-%d)
fi

sed -e "s|<release version=\"\" date=\"\" />|<release version=\"$VERSION\" date=\"$DATE\" type=\"$VERSION_TYPE\"/>|" -i $APPDIR/usr/share/metainfo/org.kde.krita.appdata.xml

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
        find $1 -type f -name "*" -not -name "*.o" -not -path "*/python3.*/*" -not -name "AppImageUpdate" -not -name "libstdc++.so.6.*" -exec sh -c '
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

EXTRA_PLUGINS_LIST="$PLUGINS,$APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so"

if [ -f $DEPS_INSTALL_PREFIX/plugins/platforms/libqwayland-generic.so ]; then
  EXTRA_PLATFORM_PLUGINS="platforms/libqwayland-generic.so,wayland-shell-integration/libxdg-shell.so,wayland-graphics-integration-client/libqt-plugin-wayland-egl.so"
  EXTRA_PLUGINS_LIST="$EXTRA_PLUGINS_LIST,$EXTRA_PLATFORM_PLUGINS"
fi

for plugin in $DEPS_INSTALL_PREFIX/plugins/wayland-decoration-client/*.so; do
  echo "Adding client-side-decoration plugin: $plugin"
  relative_plugin=`realpath -m --relative-to="$DEPS_INSTALL_PREFIX/plugins" $plugin`
  EXTRA_PLUGINS_LIST="$EXTRA_PLUGINS_LIST,$relative_plugin"
done

EXTRA_RUNTIME_ARGUMENT=

if [ -f $DEPS_INSTALL_PREFIX/appimage-tools/share/runtime-x86_64 ]; then
  EXTRA_RUNTIME_ARGUMENT=-runtime-file=$DEPS_INSTALL_PREFIX/appimage-tools/share/runtime-x86_64
fi

# Step 4: Build the image!!!
linuxdeployqt $APPDIR/usr/share/applications/org.kde.krita.desktop \
  -executable=$APPDIR/usr/bin/krita \
  ${MLT_BINARIES} \
  ${FFMPEG_BINARIES} \
  -qmldir=$KRITA_SOURCES/plugins/dockers/textproperties \
  -verbose=2 \
  -bundle-non-qt-libs \
  -extra-plugins=$EXTRA_PLUGINS_LIST \
  -updateinformation="${ZSYNC_URL}" \
  ${EXTRA_RUNTIME_ARGUMENT} \
  -appimage || (echo "failed with exit code $?"; exit 1)

# Generate a new name for the Appimage file and rename it accordingly

OLD_APPIMAGE_NAME="Krita-${VERSION}-${APPIMAGE_ARCHITECTURE}.AppImage"
mv ${OLD_APPIMAGE_NAME} ${NEW_APPIMAGE_NAME}

# TODO: when signing is implemented, make sure the package is signed **before**
#       the zsync file is generated (since singing changes the packages)

# remove the autogenerated zsync file and create the custom one
rm -f *.zsync
zsyncmake -u "${ZSYNC_SOURCE_URL}" ${NEW_APPIMAGE_NAME}
mv ${NEW_APPIMAGE_NAME}.zsync Krita-${CHANNEL}-${APPIMAGE_ARCHITECTURE}.appimage.zsync
