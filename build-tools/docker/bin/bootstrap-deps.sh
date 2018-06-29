#!/bin/bash

if [ ! -d ./persistent ]; then
    mkdir ./persistent
fi

if [ ! -f ./persistent/krita-appimage-deps.tar ]; then
    (
        cd ./persistent/
        wget https://binary-factory.kde.org/job/Krita_Nightly_Appimage_Dependency_Build/lastSuccessfulBuild/artifact/krita-appimage-deps.tar || exit 1
    )
fi

creator_major=4.6
creator_minor=2
creator_file=qt-creator-opensource-linux-x86_64-${creator_major}.${creator_minor}.run
if [ ! -f ./persistent/${creator_file} ]; then
    (
        cd ./persistent/
        wget http://download.qt.io/official_releases/qtcreator/${creator_major}/${creator_major}.${creator_minor}/${creator_file} || exit 1
        chmod a+x ${creator_file}
    )
fi
