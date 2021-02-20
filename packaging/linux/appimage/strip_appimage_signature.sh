#!/usr/bin/env bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

set -x
set -e

APPIMAGE_PATH="${1}"
CHANNEL="${2}"
GPG_KEY="${3}"

if [ -z $APPIMAGE_PATH ]; then
  echo "path to appimage (arg1) is not set"
  exit 1
fi

if [ -z $CHANNEL ]; then
  echo "channel (arg2) is not set"
  exit 1
fi

if [ -z $GPG_KEY ]; then
  echo "gpg key id (arg3) is not set"
  exit 1
fi

tempdir="$(mktemp strip_appimage_signature.XXXXXX -d -p /tmp)"

destination=$(basename $APPIMAGE_PATH)

ascfile="${tempdir}/${destination}.digest.asc"
digestfile="${tempdir}/${destination}.digest"
sigkeyfile="${tempdir}/sig_pubkey"

if [ -f $digestfile ]; then rm $digestfile; fi
if [ -f $ascfile ]; then rm $ascfile; fi
if [ -f $sigkeyfile ]; then rm $sigkeyfile; fi

# get offsets and lengths of .sha256_sig  and .sig_key sections of the AppImage
SIG_OFFSET=$(objdump -h "${APPIMAGE_PATH}" | grep .sha256_sig | awk '{print $6}')
SIG_LENGTH=$(objdump -h "${APPIMAGE_PATH}" | grep .sha256_sig | awk '{print $3}')

KEY_OFFSET=$(objdump -h "${APPIMAGE_PATH}" | grep .sig_key | awk '{print $6}')
KEY_LENGTH=$(objdump -h "${APPIMAGE_PATH}" | grep .sig_key | awk '{print $3}')

# Null the sections
dd if=/dev/zero bs=1 seek=$(($(echo 0x$SIG_OFFSET))) count=$(($(echo 0x$SIG_LENGTH))) of="${APPIMAGE_PATH}" conv=notrunc
dd if=/dev/zero bs=1 seek=$(($(echo 0x$KEY_OFFSET))) count=$(($(echo 0x$KEY_LENGTH))) of="${APPIMAGE_PATH}" conv=notrunc

# regenerate zsync file
# TODO: replace with real urls
if [ "$CHANNEL" = "Next" ]; then
	URL="http://localhost:8000/nightly"
elif [ "$CHANNEL" = "Plus" ]; then
	URL="http://localhost:8000/nightly"
elif [ "$CHANNEL" = "Stable" ]; then
	URL="http://localhost:8000/stable"
elif [ "$CHANNEL" = "Beta" ]; then
	URL="http://localhost:8000/unstable"
fi

zsyncmake -u "${URL}/$(basename ${APPIMAGE_PATH})" -o $APPIMAGE_PATH.zsync.new $APPIMAGE_PATH
ret=$?
if [ $ret -eq 0 ]; then
  mv $APPIMAGE_PATH.zsync.new Krita-${CHANNEL}-x86_64.AppImage.zsync
fi

# cleanup
rm -rf $tempdir
