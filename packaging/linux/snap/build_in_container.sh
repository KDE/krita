#!/bin/bash

set -ex

cd /workspace

ping -c1 networkcheck.kde.org

apt-key adv --keyserver keyserver.ubuntu.com --recv E6D4736255751E5D
echo 'deb http://archive.neon.kde.org/unstable bionic main' > /etc/apt/sources.list.d/neon.list
apt update

snap install --edge --classic snapcraft

snapcraft --version
snapcraft --destructive-mode

mkdir -p result
mv *.snap result/
