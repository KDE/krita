#!/usr/bin/env bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

set -x
set -e

APPIMAGE_PATH="${1}"
GPG_KEY="${2}"

if [ -z $APPIMAGE_PATH ]; then
  echo "path to appimage (arg1) is not set"
  exit 1
fi

if [ -z $GPG_KEY ]; then
  echo "gpg key id (arg3) is not set"
  exit 1
fi

tempdir="$(mktemp sign_appimage.XXXXXX -d -p /tmp)"

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

# generate sha256sum
# BEWARE THE NEWLINE! if it is not stripped, AppImageUpdate validaton will fail
sha256sum $APPIMAGE_PATH | cut -d " " -f 1 | tr -d '\n' > $digestfile

#sign the sha256sum
gpg2 --detach-sign --armor -u $GPG_KEY -o $ascfile $digestfile
gpg2 --export --armor $GPG_KEY > $sigkeyfile

# Embed the signature
dd if=${ascfile} bs=1 seek=$(($(echo 0x$SIG_OFFSET))) count=$(($(echo 0x$SIG_LENGTH))) of="${APPIMAGE_PATH}" conv=notrunc
# Embed the public part of the signing key
dd if=${sigkeyfile} bs=1 seek=$(($(echo 0x$KEY_OFFSET))) count=$(($(echo 0x$KEY_LENGTH))) of="${APPIMAGE_PATH}" conv=notrunc

# cleanup
rm -rf $tempdir
