#!/usr/bin/env zsh
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

if test -z $BUILDROOT; then
    echo "ERROR: BUILDROOT env not set, exiting!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    exit 1
fi

BUILDROOT="${BUILDROOT%/}"
echo "BUILDROOT set to ${BUILDROOT}"

if [[ -z "${KIS_SRC_DIR}" ]]; then
    KIS_SRC_DIR=${BUILDROOT}/krita
fi
if [[ -z "${KIS_BUILD_DIR}" ]]; then
    KIS_BUILD_DIR=${BUILDROOT}/kisbuild
fi


ENTITLEMENTS="sandboxdev-entitlements.plist"
if [[ -n "${1}" ]]; then
    ENTITLEMENTS="entitlements.plist"
fi

codesign_bin() {
    codesign --options runtime --timestamp -f -s "${APPLEDEV_IDAPP}" \
 --entitlements "${KIS_BUILD_DIR}/packaging/macos/${2}" "${1}"
}


codesign_sandbox() {
    echo "${1}"
    codesign_bin "${1}" "${2}"
    codesign -d --entitlements - "${1}"
}


# program starts
# codesign_sandbox "${BUILDROOT}/i/bin/krita.app/Contents/container-migration.plist"
codesign_sandbox "${BUILDROOT}/i/bin/krita.app/Contents/MacOS/ffprobe" "sandboxdev_sub-entitlements.plist"
codesign_sandbox "${BUILDROOT}/i/bin/krita.app/Contents/MacOS/ffmpeg" "sandboxdev_sub-entitlements.plist"
codesign_sandbox "${BUILDROOT}/i/bin/krita.app/Contents/MacOS/krita" "${ENTITLEMENTS}"

echo "codesign end."
