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

