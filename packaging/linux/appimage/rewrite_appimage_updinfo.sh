#!/usr/bin/env bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

set -x
set -e

APPIMAGE_PATH="${1}"
UPDURL="${2}"

if [ -z $APPIMAGE_PATH ]; then
  echo "path to appimage (arg1) is not set"
  exit 1
fi

if [ -z $UPDURL ]; then
  echo "update url (arg2) is not set"
  exit 1
fi


tempdir="$(mktemp rewrite_appimage_updinfo.XXXXXX -d -p /tmp)"

destination=$(basename $APPIMAGE_PATH)
updinfo_file="${tempdir}/upd_info"

echo -n "zsync|${UPDURL}" > $updinfo_file

# get offsets and lengths of .upd_info section of the AppImage
OFFSET=$(objdump -h "${APPIMAGE_PATH}" | grep .upd_info | awk '{print $6}')
LENGTH=$(objdump -h "${APPIMAGE_PATH}" | grep .upd_info | awk '{print $3}')

# Null the section
dd if=/dev/zero bs=1 seek=$(($(echo 0x$OFFSET))) count=$(($(echo 0x$LENGTH))) of="${APPIMAGE_PATH}" conv=notrunc

# Embed the updinfo
dd if=${updinfo_file} bs=1 seek=$(($(echo 0x$OFFSET))) count=$(($(echo 0x$LENGTH))) of="${APPIMAGE_PATH}" conv=notrunc

# cleanup
rm -rf $tempdir
