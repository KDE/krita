#!/usr/bin/env bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

set -x
set -e

APPIMAGE_PATH="${1}"

if [ -z $APPIMAGE_PATH ]; then
  echo "path to appimage (arg1) is not set"
  exit 1
fi

tempdir="$(mktemp validate_appimage_signature.XXXXXX -d -p /tmp)"

destination=$(basename $APPIMAGE_PATH)

ascfile="${tempdir}/${destination}.digest.asc"
digestfile="${tempdir}/${destination}.digest"
sigkeyfile="${tempdir}/sig_pubkey"
tempkeyringpath="${tempdir}/keyring"
tmpappimage="$tempdir/tmp.AppImage"

# get offsets and lengths of .sha256_sig  and .sig_key sections of the AppImage
SIG_OFFSET=$(objdump -h "${APPIMAGE_PATH}" | grep .sha256_sig | awk '{print $6}')
SIG_LENGTH=$(objdump -h "${APPIMAGE_PATH}" | grep .sha256_sig | awk '{print $3}')

KEY_OFFSET=$(objdump -h "${APPIMAGE_PATH}" | grep .sig_key | awk '{print $6}')
KEY_LENGTH=$(objdump -h "${APPIMAGE_PATH}" | grep .sig_key | awk '{print $3}')

cp $APPIMAGE_PATH $tmpappimage

# restore the original, for generating checksum
dd if=/dev/zero bs=1 seek=$(($(echo 0x$SIG_OFFSET))) count=$(($(echo 0x$SIG_LENGTH))) of="${tmpappimage}" conv=notrunc
dd if=/dev/zero bs=1 seek=$(($(echo 0x$KEY_OFFSET))) count=$(($(echo 0x$KEY_LENGTH))) of="${tmpappimage}" conv=notrunc

sha256sum $tmpappimage | cut -d " " -f 1 | tr -d '\n' > $digestfile

# extract signature
dd if="${APPIMAGE_PATH}" bs=1 skip=$(($(echo 0x$SIG_OFFSET))) count=$(($(echo 0x$SIG_LENGTH))) of="${ascfile}"
# extract the public part of the signing key
dd if=${APPIMAGE_PATH} bs=1 skip=$(($(echo 0x$KEY_OFFSET))) count=$(($(echo 0x$KEY_LENGTH))) of="${sigkeyfile}"

cat $sigkeyfile | gpg2 --no-default-keyring --keyring $tempkeyringpath  --import
gpg2 --no-default-keyring --keyring $tempkeyringpath --verify $ascfile $digestfile

# cleanup
rm -rf $tempdir
