#!/bin/bash

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
export DEPS_INSTALL_PREFIX=$BUILD_PREFIX/deps/usr/
export DOWNLOADS_DIR=$BUILD_PREFIX/downloads/

# Setup variables needed to help everything find what we built
export LD_LIBRARY_PATH=$DEPS_INSTALL_PREFIX/lib/:$DEPS_INSTALL_PREFIX/lib/x86_64-linux-gnu/:$APPDIR/usr/lib/:$LD_LIBRARY_PATH
export PATH=$DEPS_INSTALL_PREFIX/bin/:$PATH
export PKG_CONFIG_PATH=$DEPS_INSTALL_PREFIX/share/pkgconfig/:$DEPS_INSTALL_PREFIX/lib/pkgconfig/:/usr/lib/pkgconfig/:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$DEPS_INSTALL_PREFIX:$CMAKE_PREFIX_PATH
export PYTHONPATH=$DEPS_INSTALL_PREFIX/sip/:$DEPS_INSTALL_PREFIX/lib/python3.8/site-packages/:$DEPS_INSTALL_PREFIX/lib/python3.8/
export PYTHONHOME=$DEPS_INSTALL_PREFIX

# download
mkdir -p $DOWNLOADS_DIR
cd $DOWNLOADS_DIR
wget "https://files.kde.org/krita/build/AppImageUpdate-x86_64.AppImage" -O AppImageUpdate
echo -n "ebc4763e8eac6aa7b9dfcbea77ec07d2e01fa1b9f10a38d4af0fc040bc965c1f AppImageUpdate" | sha256sum -c -

# Switch over to our build prefix
cd $BUILD_PREFIX

#
# Now we can get the process started!
#

# Step 0: place the translations where ki18n and Qt look for them
if [ -d $APPDIR/usr/share/locale ] ; then
    mv $APPDIR/usr/share/locale $APPDIR/usr/share/krita
fi

# Step 1: Copy over all the resources provided by dependencies that we need
cp -r $DEPS_INSTALL_PREFIX/share/locale $APPDIR/usr/share/krita
cp -r $DEPS_INSTALL_PREFIX/share/kf5 $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/share/mime $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/lib/python3.8 $APPDIR/usr/lib
cp -r $DEPS_INSTALL_PREFIX/share/sip $APPDIR/usr/share
cp -r $DEPS_INSTALL_PREFIX/translations $APPDIR/usr/

# Step 2: Relocate x64 binaries from the architecture specific directory as required for Appimages
mv $APPDIR/usr/lib/x86_64-linux-gnu/*  $APPDIR/usr/lib
rm -rf $APPDIR/usr/lib/x86_64-linux-gnu/

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
patchelf --set-rpath '$ORIGIN/../..' $APPDIR/usr/lib/python3.8/site-packages/PyQt5/sip.so


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
if [[ -d .git ]]; then
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

    #if KRITA_BETA is set, set channel to Beta, otherwise set it to stable
    grep "define KRITA_BETA 1" libs/version/kritaversion.h;
    is_beta=$?

    grep "define KRITA_RC 1" libs/version/kritaversion.h;
    is_rc=$?

    if [ is_beta -eq 0 ]; then
        VERSION_TYPE="development"
    else
        VERSION_TYPE="stable"
    fi

    if [ -z "${CHANNEL}" ]; then
        if [ is_beta -eq 0 -o is_rc -eq 0 ]; then
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

# Step 4: Build the image!!!
linuxdeployqt $APPDIR/usr/share/applications/org.kde.krita.desktop \
  -executable=$APPDIR/usr/bin/krita \
  -qmldir=$DEPS_INSTALL_PREFIX/qml \
  -verbose=2 \
  -bundle-non-qt-libs \
  -extra-plugins=$PLUGINS,$APPDIR/usr/lib/krita-python-libs/PyKrita/krita.so,$APPDIR/usr/lib//qml/org/krita/sketch/libkritasketchplugin.so,$APPDIR/usr/lib/qml/org/krita/draganddrop/libdraganddropplugin.so  \
  -updateinformation="${ZSYNC_URL}" \
  -appimage


OLD_APPIMAGE_NAME="Krita-${VERSION}-x86_64.AppImage"
NEW_APPIMAGE_NAME="krita-${VERSION}-x86_64.appimage"
mv ${OLD_APPIMAGE_NAME} ${NEW_APPIMAGE_NAME}
