#!/usr/bin/env zsh
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# macstoreprep
# 
# Script to prepare a dmg for mac Store submission format
#

if [[ -n "$ZSH_VERSION" ]]; then
    emulate -L ksh;
fi

if [[ -z $BUILDROOT ]]; then
    echo "ERROR: BUILDROOT env not set!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    echo "exiting..."
    exit 1
fi

# receives a path without filename
get_absolute_path() {
    local origLoc="$(pwd)"

    local filename="${1}"
    local location="${filename}"

    if [[ -e "${filename}" && ! -d "${filename}" ]]; then
        location="$(dirname "${filename}")"
    fi

    local absolute="$(cd "${location}"; pwd)"
    cd "${origLoc}"

    echo ${absolute}
}

# make sure BUILDROOT is always an absolute path
BUILDROOT=$(get_absolute_path "${BUILDROOT}")

KRITADMG_WDIR="${BUILDROOT}/kritadmg_store"
KRITADMG_MOUNT="kritadmg"
if [[ -z "${KIS_SRC_DIR}" ]]; then
    KIS_SRC_DIR=${BUILDROOT}/krita
fi
if [[ -z "${KIS_BUILD_DIR}" ]]; then
    KIS_BUILD_DIR=${BUILDROOT}/kisbuild
fi
KIS_ENTITLEMENTS_DIR="${KIS_SRC_DIR}/packaging/macos/"
# we look for the provisioning file in the script the directory was called
KIS_PROVISION="$(get_absolute_path ".")/embedded.provisionprofile"

KIS_BUNDLE_ID="org.kde.krita"

# print status messages
print_msg() {
    printf "\e[32m${1}\e[0m\n" "${@:2}"
    # printf "%s\n" "${1}" >> ${OUTPUT_LOG}
}

# print error
print_error() {
    printf "\e[31m%s %s\e[0m\n" "Error:" "${1}"
}

get_script_dir() {
    if [[ -n "$ZSH_VERSION" ]]; then
        script_source="${(%):-%x}"
    else
        # transitional, macos should use ZSH
        script_source="${BASH_SOURCE[0]}"
    fi
    # go to target until finding root.
    while [ -L "${script_source}" ]; do
        script_target="$(readlink ${script_source})"
        if [[ "${script_source}" = /* ]]; then
            script_source="$script_target"
        else
            script_dir="$(dirname "${script_source}")"
            script_source="${script_dir}/${script_target}"
        fi
    done
    echo "$(dirname ${script_source})"
}

findEntitlementsFile() {
    if [[ -e ${KIS_ENTITLEMENTS} ]]; then
        return
    fi

    local fileName="macStore-entitlements.plist"
    local subEntitlementsFile="sandboxdev_sub-entitlements.plist"

    if [[ -f "${DIR_CURRENT}/${fileName}" ]]; then
        KIS_ENTITLEMENTS="${DIR_CURRENT}/${fileName}"
        KIS_SUB_ENTITLEMENTS="${DIR_CURRENT}/${subEntitlementsFile}"
    elif [[ -f "${KIS_ENTITLEMENTS_DIR}/${fileName}" ]]; then
        KIS_ENTITLEMENTS="${KIS_ENTITLEMENTS_DIR}/${fileName}"
        KIS_SUB_ENTITLEMENTS="${KIS_ENTITLEMENTS_DIR}/${subEntitlementsFile}"
    fi
}

# calls /usr/libexec/PlistBuddy with basic args
# $1 file.plist to modify
# $2 -c command
# $3 plist key to modify
# $4 plist value for key
plistbuddy() {
    local plistbuddy="/usr/libexec/PlistBuddy"
    local filename="${1}"
    local cmd="${2}"
    local key="${3}"
    local value="${4}"
    
    ${plistbuddy} "${filename}" -c "${cmd}:${key} ${value}"
}

modifyInfoPlistContents() {
    local plistbuddy="/usr/libexec/PlistBuddy"
    local PLIST_LOC="${KRITADMG_WDIR}/krita.app/Contents"
    
    plistbuddy "${PLIST_LOC}/Info.plist" Set CFBundleIdentifier ${KIS_BUNDLE_ID}
    plistbuddy "${PLIST_LOC}/Info.plist" Set CFBundleVersion ${KIS_BUILD_VERSION}
    plistbuddy "${PLIST_LOC}/Info.plist" Set CFBundleShortVersionString ${KIS_VERSION}
    plistbuddy "${PLIST_LOC}/Info.plist" CFBundleLongVersionString ${KIS_VERSION}

    plistbuddy "${PLIST_LOC}/Library/QuickLook/kritaquicklook.qlgenerator/Contents/Info.plist" CFBundleIdentifier ${KIS_BUNDLE_ID}.quicklook
    plistbuddy "${PLIST_LOC}/Library/Spotlight/kritaspotlight.mdimporter/Contents/Info.plist" CFBundleIdentifier ${KIS_BUNDLE_ID}.spotlight
}


clean_dmg() {
    cd "${KRITADMG_WDIR}/krita.app/Contents/MacOS"
    rm krita_version
    rm kritarunner
}

batch_codesign() {
    local entitlements="${1}"
    if [[ -z "${1}" ]]; then
        entitlements="${KIS_ENTITLEMENTS}"
    fi
    xargs -P4 -I FILE codesign --options runtime --timestamp -f -s "${CODE_SIGNATURE}" --entitlements "${entitlements}" FILE
}
# Code sign must be done as recommended by apple "sign code inside out in individual stages"
signBundle() {
    cd ${KRITADMG_WDIR}

    # sign Frameworks and libs
    cd ${KRITADMG_WDIR}/krita.app/Contents/Frameworks
    # remove debug version as both versions can't be signed.
    rm ${KRITADMG_WDIR}/krita.app/Contents/Frameworks/QtScript.framework/Versions/Current/QtScript_debug
    # Do not sign binaries inside frameworks except for Python's
    find . -type d -path "*.framework" -prune -false -o -perm +111 -not -type d | batch_codesign
    find Python.framework -type f -name "*.o" -or -name "*.so" -or -perm +111 -not -type d -not -type l | batch_codesign
    find . -type d -name "*.framework" | xargs printf "%s/Versions/Current\n" | batch_codesign

    # Sign all other files in Framework (needed)
    # there are many files in python do we need to sign them all?
    find krita-python-libs -type f | batch_codesign
    # find python -type f | batch_codesign

    # Sign only libraries and plugins
    cd ${KRITADMG_WDIR}/krita.app/Contents/PlugIns
    find . -type f | batch_codesign

    cd ${KRITADMG_WDIR}/krita.app/Contents/Library/QuickLook
    printf "kritaquicklook.qlgenerator" | batch_codesign

    cd ${KRITADMG_WDIR}/krita.app/Contents/Library/Spotlight
    printf "kritaspotlight.mdimporter" | batch_codesign

    # It is necessary to sign every binary Resource file
    cd ${KRITADMG_WDIR}/krita.app/Contents/Resources
    find . -perm +111 -type f | batch_codesign

    printf "${KRITADMG_WDIR}/krita.app/Contents/MacOS/ffmpeg" | batch_codesign "${KIS_SUB_ENTITLEMENTS}"
    printf "${KRITADMG_WDIR}/krita.app/Contents/MacOS/ffprobe" | batch_codesign "${KIS_SUB_ENTITLEMENTS}"
    #Finally sign krita and krita.app
    printf "${KRITADMG_WDIR}/krita.app/Contents/MacOS/krita" | batch_codesign
    printf "${KRITADMG_WDIR}/krita.app" | batch_codesign

    cd ${KRITADMG_WDIR}
}

sign_hasError() {
    local CODESIGN_STATUS=0
    for f in $(find "${KRITADMG_WDIR}/krita.app" -type f); do
        if [[ -z $(file ${f} | grep "Mach-O") ]]; then
            continue
        fi

        CODESIGN_RESULT=$(codesign -vvv --strict ${f} 2>&1 | grep "not signed")

        if [[ -n "${CODESIGN_RESULT}" ]]; then
            CODESIGN_STATUS=1
            printf "${f} not signed\n" >&2
        fi
    done
    echo ${CODESIGN_STATUS}
}


print_usage () {
    printf "USAGE:
  macstoreprep.sh -f=<krita.dmg> [-s=<identity>] [-notarize-ac=<apple-account>]

    -bv \t\t bundle version: mac store needs to identify each submission as unique. for this
        \t\t we do not bump the krita version but the bundle version. The format is [M.m.p] and
        \t\t is totally independent from krita release version.

    -s \t\t\t Sign Identity for 3rd party application

    -sins \t\t Sign Identity for 3rd party Dev installer

    -notarize-ac \t Apple account name for notarization purposes
        \t\t script will attempt to get password from keychain, if fails provide one with
        \t\t the -notarize-pass option: To add a password run

        \t\t security add-generic-password -a \"AC_USERNAME\" -w <secret_password> -s \"AC_PASSWORD\"

    -notarize-pass \t If given, the Apple account password. Otherwise an attempt will be macdeployqt_exists
        \t\t to get the password from keychain using the account given in <notarize-ac> option.

    -asc-provider \t some AppleIds might need this option pass the <shortname>

    -f \t\t\t input dmg
"
}


### Script starts
SCRIPT_SOURCE_DIR="$(get_script_dir)"

DIR_CURRENT="$(pwd)"
cd "$(get_script_dir)"
SCRIPT_SOURCE_DIR="$(pwd)"
cd "${DIR_CURRENT}"

# Attempt to detach previous mouted DMG
# if [[ -d "/Volumes/${DMG_title}" ]]; then
#     echo "WARNING: Another Krita DMG is mounted!"
#     echo "Attempting eject…"
#     hdiutil detach "/Volumes/${DMG_title}"
#     if [ $? -ne 0  ]; then
#         exit 1
#     fi
#     echo "Success!"
# fi


# -- Parse input args
for arg in "${@}"; do
    # If string starts with -sign
#    if [[ ${arg} = -v=* ]]; then
#        KIS_VERSION="${arg#*=}"
#    fi

    if [[ ${arg} = -bv=* ]]; then
        KIS_BUILD_VERSION="${arg#*=}"
    fi

    if [[ ${arg} = -s=* ]]; then
        CODE_SIGNATURE="${arg#*=}"
    fi

    if [[ ${arg} = -sins=* ]]; then
        SIGN_DEV_INSTALL="${arg#*=}"
    fi

    if [[ ${arg} = -f=* ]]; then
        INPUT_STR="${arg#*=}"
        INPUT_DIR="$(cd $(dirname ${INPUT_STR}); pwd -P)"
        INPUT_DMG="${INPUT_DIR}/$(basename ${INPUT_STR})"
    fi

    if [[ ${arg} = -notarize-ac=* ]]; then
        NOTARIZE_ACC="${arg#*=}"
    fi

    if [[ ${arg} = -notarize-pass=* ]]; then
        NOTARIZE_PASS="${arg#*=}"
    fi

    if [[ ${arg} = -asc-provider=* ]]; then
        APPLE_TEAMID="${arg#*=}"
    fi

    if [[ ${arg} = "-h" || ${arg} = "--help" ]]; then
        print_usage
        exit 1
    fi
done

### Code Signature & NOTARIZATION
NOTARIZE="false"
if [[ -n "${CODE_SIGNATURE}" ]]; then
    print_msg "Code will be signed with %s" "${CODE_SIGNATURE}"
fi

if [[ -z "${KIS_BUILD_VERSION}" ]]; then
    print_error "option -bv is not set!"
    echo "run with --help for assistance"
    exit
fi

# mount dmg

if [[ ! -d "${KRITADMG_WDIR}" ]]; then
    mkdir "${KRITADMG_WDIR}"
fi

cd "${KRITADMG_WDIR}"
echo "cleaning last run..."
rm -rf "${KRITADMG_WDIR}/krita.app"

echo "mounting ${INPUT_DMG} ..."
hdiutil attach "${INPUT_DMG}" -mountpoint "${KRITADMG_MOUNT}"
rsync -prult --delete "${KRITADMG_MOUNT}/krita.app/" "krita.app"

hdiutil detach "${KRITADMG_MOUNT}"

# attach provisioning profile
cp -X "${KIS_PROVISION}" "${KRITADMG_WDIR}/krita.app/Contents/"
chmod 644 "${KRITADMG_WDIR}/krita.app/Contents/${KIS_PROVISION}"

# clean krita version string (app store format does not allow dashes
KIS_VERSION="$(${KRITADMG_WDIR}/krita.app/Contents/MacOS/krita_version 2> /dev/null | awk '{print $1}')"
KIS_VERSION=${KIS_VERSION%%-*}

# try to fix permissions
for f in $(find ${KRITADMG_WDIR} -not -perm +044); do
    chmod +r ${f}
done

rootfiles="$(find ${KRITADMG_WDIR} -user root -perm +400 -and -not -perm +044)"
if [[ -n ${rootfiles} ]]; then
    echo "files \n ${rootfiles} \n cannot be read by non-root users!"
    echo "submission will fail, please fix and relaunch script"
fi

# replace signature
findEntitlementsFile
if [[ -z ${KIS_ENTITLEMENTS} ]]; then
    echo "Could not find entitlements file use for codesign"
    exit
else
    echo "using ${KIS_ENTITLEMENTS}"
fi

modifyInfoPlistContents

clean_dmg

# Code sign krita.app if signature given
if [[ -n "${CODE_SIGNATURE}" ]]; then
    signBundle
    if [[ $(sign_hasError) -eq 1 ]]; then
        print_error "CodeSign errors!"
        echo "stopping...."
        exit 1
    fi
    echo "no codesign errors"
    echo "preparing pkg"
fi

if [[ -z ${APPLE_TEAMID} ]]; then
    APPLE_TEAMID="<team_identifier>"
fi

KRITA_DMG_BASENAME="$(basename ${INPUT_STR})"
KRITA_PKG_NAME="${KRITA_DMG_BASENAME%.*}.pkg"
# productbuild --component ${KIS_APPLOC}/krita.app  /Applications krita5_submit.pkg --sign \"3rd Party Mac Developer Installer: ...\" 
if [[ -n "${SIGN_DEV_INSTALL}" ]]; then
    echo "creating krita5_submit.pkg ..."
    productbuild --component "${KRITADMG_WDIR}/krita.app"  "/Applications" "${KRITADMG_WDIR}/${KRITA_PKG_NAME}" --sign "${SIGN_DEV_INSTALL}"
fi

echo "done"

print_msg "
Use altool to submit the Pkg:

    \t xcrun altool --upload-package "${KRITADMG_WDIR}/${KRITA_PKG_NAME}" --bundle-id ${KIS_BUNDLE_ID} -t macos -u "${NOTARIZE_ACC}" --password <pass> [--asc-provider "${APPLE_TEAMID}"] --bundle-version ${KIS_BUILD_VERSION} --bundle-short-version-string ${KIS_VERSION} --apple-id <app-id_fromStore>

To get <app-id_fromStore> from store go to: https://appstoreconnect.apple.com/apps ,select the app you wish to upload to,
click on App information on the left side, and search for 'Apple ID' field.

\n"
