#!/usr/bin/env bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

set -x
set -e

APPIMAGE_PATH="${1}"
CHANNEL="${2}"
VERSION="${3}"

if [ -z $APPIMAGE_PATH ]; then
  echo "path to appimage (arg1) is not set"
  exit 1
fi

if [ -z $CHANNEL ]; then
  echo "channel (arg2) is not set"
  exit 1
fi

if [ -z $VERSION ]; then
  echo "version (arg3) is not set"
  exit 1
fi

# regenerate zsync file
if [ "$CHANNEL" = "Next" ]; then
	URL="${BUILD_URL}/artifact/"
elif [ "$CHANNEL" = "Plus" ]; then
	URL="${BUILD_URL}/artifact/"
elif [ "$CHANNEL" = "Stable" ]; then
	URL="https://download.kde.org/stable/krita/${VERSION}"
elif [ "$CHANNEL" = "Beta" ]; then
	URL="https://download.kde.org/unstable/krita/${VERSION}"
fi

zsyncmake -u "${URL}/$(basename ${APPIMAGE_PATH})" -o $APPIMAGE_PATH.zsync.new $APPIMAGE_PATH
ret=$?
if [ $ret -eq 0 ]; then
  mv $APPIMAGE_PATH.zsync.new Krita-${CHANNEL}-x86_64.appimage.zsync
fi

