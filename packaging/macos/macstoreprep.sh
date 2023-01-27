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

BUILDROOT="${BUILDROOT%/}"

KRITADMG_WDIR="${BUILDROOT}/kritadmg_store"
KRITADMG_MOUNT="kritadmg"

# print status messages
print_msg() {
    printf "\e[32m${1}\e[0m\n" "${@:2}"
    # printf "%s\n" "${1}" >> ${OUPUT_LOG}
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
    if [[ -f "${DIR_CURRENT}/${fileName}" ]]; then
        KIS_ENTITLEMENTS="${DIR_CURRENT}/${fileName}"
    elif [[ -f "${SCRIPT_SOURCE_DIR}/${fileName}" ]]; then
        KIS_ENTITLEMENTS="${SCRIPT_SOURCE_DIR}/${fileName}"
    fi
}


modifyInfoPlistContents() {
    local LAST_LOC=$(pwd)
    cd "${KRITADMG_WDIR}/krita.app/Contents"
    sed -i '' -e 's/org.krita/org.kde.krita/g' Info.plist
    sed -i '' -e "s/org.krita.quicklook/org.kde.krita.quicklook/g" Library/QuickLook/kritaquicklook.qlgenerator/Contents/Info.plist
    sed -i '' -e "s/org.krita.spotlight/org.kde.krita.spotlight/g" Library/Spotlight/kritaspotlight.mdimporter/Contents/Info.plist
    cd  "${LAST_LOC}"
}


batch_codesign() {
    xargs -P$(sysctl -n hw.logicalcpu) -I FILE codesign --options runtime --timestamp -f -s "${CODE_SIGNATURE}" --entitlements "${KIS_ENTITLEMENTS}" FILE
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

    -s \t\t\t Sign Identity for 3rd party application

    -sins \t\t\t Sign Identity for 3rd party Dev installer

    -notarize-ac \t Apple account name for notarization purposes
\t\t\t script will attempt to get password from keychain, if fails provide one with
\t\t\t the -notarize-pass option: To add a password run

\t\t\t   security add-generic-password -a \"AC_USERNAME\" -w <secret_password> -s \"AC_PASSWORD\"

    -notarize-pass \t If given, the Apple account password. Otherwise an attempt will be macdeployqt_exists
\t\t\t to get the password from keychain using the account given in <notarize-ac> option.

    -asc-provider \t some AppleIds might need this option pass the <shortname>

\t\t\t macstoreprep needs an input dmg
"
}


### Script starts
SCRIPT_SOURCE_DIR="$(get_script_dir)"

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


# mount dmg

if [[ ! -d "${KRITADMG_WDIR}" ]]; then
    mkdir "${KRITADMG_WDIR}"
fi

cd "${KRITADMG_WDIR}"
echo "mounting ${INPUT_DMG} ..."
hdiutil attach "${INPUT_DMG}" -mountpoint "${KRITADMG_MOUNT}"
rsync -prul --delete "${KRITADMG_MOUNT}/krita.app/" "krita.app"

hdiutil detach "${KRITADMG_MOUNT}"

# replace signature
findEntitlementsFile
if [[ -z ${KIS_ENTITLEMENTS} ]]; then
    echo "Could not find entitlements file use for codesign"
    exit
else
    echo "using ${KIS_ENTITLEMENTS}"
fi

modifyInfoPlistContents

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

# productbuild --component ${KIS_APPLOC}/krita.app  /Applications krita5_submit.pkg --sign \"3rd Party Mac Developer Installer: ...\" 
if [[ -n "${SIGN_DEV_INSTALL}" ]]; then
    echo "creating krita5_submit.pkg ..."
    productbuild --component "${KRITADMG_WDIR}/krita.app"  "/Applications" "${KRITADMG_WDIR}/krita5_submit.pkg" --sign "${SIGN_DEV_INSTALL}"
fi

echo "done"

print_msg "
Use altool to submit the Pkg:

     \t xcrun altool --upload-package krita5_submit.pkg --bundle-id <bundle-id> -t macos -u <appstore-username> --password <pass> [--asc-provider <teamid>] --bundle-version <version> --bundle-short-version-string <version> --apple-id <app-id_fromStore>

\n"
