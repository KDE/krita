#!/bin/bash

# exit when any command fails
set -e

KRITA_VERSION=4.4.0-beta2

WIN64_ARTIFACTS_URL=https://binary-factory.kde.org/job/Krita_Release_Windows64_Build/lastSuccessfulBuild/artifact

WIN32_ARTIFACTS_URL=https://binary-factory.kde.org/job/Krita_Release_Windows32_Build/lastSuccessfulBuild/artifact

APPIMAGE_ARTIFACTS_URL=https://binary-factory.kde.org/job/Krita_Release_Appimage_Build/lastSuccessfulBuild/artifact

OSX_ARTIFACTS_URL=https://binary-factory.kde.org/job/Krita_Release_MacOS_Build/lastSuccessfulBuild/artifact

wget $WIN64_ARTIFACTS_URL/krita-x64-$KRITA_VERSION-dbg.zip
wget $WIN64_ARTIFACTS_URL/krita-x64-$KRITA_VERSION.zip
wget $WIN64_ARTIFACTS_URL/krita-x64-$KRITA_VERSION-setup.exe

wget $WIN32_ARTIFACTS_URL/krita-x86-$KRITA_VERSION-dbg.zip
wget $WIN32_ARTIFACTS_URL/krita-x86-$KRITA_VERSION.zip
wget $WIN32_ARTIFACTS_URL/krita-x86-$KRITA_VERSION-setup.exe

wget $APPIMAGE_ARTIFACTS_URL/gmic_krita_qt-x86_64.appimage
wget $APPIMAGE_ARTIFACTS_URL/krita-$KRITA_VERSION-x86_64.appimage
# only for Krita 5.0.0
#wget $APPIMAGE_ARTIFACTS_URL/Krita-CHANGEME-x86_64.appimage.zsync

if [[ -f archive.zip ]]; then
   rm archive.zip
fi

if [[ -d archive ]]; then
   rm -d archive
fi

wget $OSX_ARTIFACTS_URL/*zip*/archive.zip
unzip archive.zip
mv archive/krita-nightly_*.dmg krita-$KRITA_VERSION.dmg
rm archive.zip
rm -rf archive
