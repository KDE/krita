#!/usr/bin/env bash
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

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
    script_source="${BASH_SOURCE[0]}"
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

# Attempt to detach previous mouted DMG
detachKritaDMG() {
    if [[ -d "/Volumes/${DMG_title}" ]]; then
    echo "WARNING: Another Krita DMG is mounted!"
    echo "Attempting eject…"
    hdiutil detach "/Volumes/${DMG_title}"
    if [ $? -ne 0  ]; then
        exit 1
    fi
    echo "Success!"
    fi
}

print_usage () {
    printf "USAGE:
  changeSignature.sh -f=<file.dmg>[-s=<identity>] [-notarize-ac=<apple-account>] [-style=<style.txt>] [-bg=<background-image>]

    -f \t\t\t Krita source dmg to covert prepare for Pkg

    -e \t\t\t Optional entitlements file, by default it will resign using macStore-entitlements.plist
\t\t\† from packaging/macos.

    -s \t\t\t Code sign identity for codesign

    -notarize-ac \t Apple account name for notarization purposes
\t\t\t script will attempt to get password from keychain, if fails provide one with
\t\t\t the -notarize-pass option: To add a password run

\t\t\t   security add-generic-password -a \"AC_USERNAME\" -w <secret_password> -s \"AC_PASSWORD\"

    -notarize-pass \t If given, the Apple account password. Otherwise an attempt will be macdeployqt_exists
\t\t\t to get the password from keychain using the account given in <notarize-ac> option.

    -asc-provider \t some AppleIds might need this option pass the <shortname>

    -name \t\t Set the DMG name output.

\t\t\t osxdeploy needs an input image to attach to the dmg background
\t\t\t image recommended size is at least 950x500
"
}


# Helper functions
countArgs () {
    echo "${#}"
}

stringContains () {
    echo "$(grep "${2}" <<< "${1}")"
}


# helper to define function only once
batch_codesign() {
    xargs -P$(sysctl -n hw.logicalcpu) -I FILE codesign --options runtime --timestamp -f -s "${CODE_SIGNATURE}" --entitlements "${KIS_ENTITLEMENTS}" FILE
}
# Code sign must be done as recommended by apple "sign code inside out in individual stages"
signBundle() {
    cd ${KIS_APPLOC}

    # sign Frameworks and libs
    cd ${KIS_APPLOC}/krita.app/Contents/Frameworks
    # remove debug version as both versions can't be signed.
    rm ${KIS_APPLOC}/krita.app/Contents/Frameworks/QtScript.framework/Versions/Current/QtScript_debug
    # Do not sign binaries inside frameworks except for Python's
    find . -type d -path "*.framework" -prune -false -o -perm +111 -not -type d | batch_codesign
    find Python.framework -type f -name "*.o" -or -name "*.so" -or -perm +111 -not -type d -not -type l | batch_codesign
    find . -type d -name "*.framework" | xargs printf "%s/Versions/Current\n" | batch_codesign

    # Sign all other files in Framework (needed)
    # there are many files in python do we need to sign them all?
    find krita-python-libs -type f | batch_codesign
    # find python -type f | batch_codesign

    # Sign only libraries and plugins
    cd ${KIS_APPLOC}/krita.app/Contents/PlugIns
    find . -type f | batch_codesign

    cd ${KIS_APPLOC}/krita.app/Contents/Library/QuickLook
    printf "kritaquicklook.qlgenerator" | batch_codesign

    cd ${KIS_APPLOC}/krita.app/Contents/Library/Spotlight
    printf "kritaspotlight.mdimporter" | batch_codesign

    # It is necessary to sign every binary Resource file
    cd ${KIS_APPLOC}/krita.app/Contents/Resources
    find . -perm +111 -type f | batch_codesign

    #Finally sign krita and krita.app
    printf "${KIS_APPLOC}/krita.app/Contents/MacOS/krita" | batch_codesign
    printf "${KIS_APPLOC}/krita.app" | batch_codesign
}

sign_hasError() {
    local CODESIGN_STATUS=0
    for f in $(find "${KIS_APPLOC}" -type f); do
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

copyApplicationFolder() {
    mkdir "${KIS_APPLOC}"

    if [[ ${DMG_INPUT_FILE##*.} = "dmg" ]]; then
        detachKritaDMG

        hdiutil attach "${DMG_INPUT_FILE}"
        rsync -a "/Volumes/krita/krita.app" "${KIS_APPLOC}/"
        hdiutil detach "/Volumes/krita"

    elif [[ ${DMG_INPUT_FILE##*.} = "app" ]]; then
        rsync -a "${DMG_INPUT_FILE}" "${KIS_APPLOC}/"
    fi

}

modifyInfoPlistContents() {
    cd "${KIS_APPLOC}/krita.app/Contents"
    sed -i '' -e 's/org.krita/org.kde.krita/g' Info.plist
    sed -i '' -e "s/org.krita.quicklook/org.kde.krita.quicklook/g" Library/QuickLook/kritaquicklook.qlgenerator/Contents/Info.plist
    sed -i '' -e "s/org.krita.spotlight/org.kde.krita.spotlight/g" Library/Spotlight/kritaspotlight.mdimporter/Contents/Info.plist
}



#######################
# Program starts!!
########################
DMG_title="krita" #if changed krita.temp.dmg must be deleted manually

DIR_CURRENT="$(pwd)"
cd "$(get_script_dir)"
SCRIPT_SOURCE_DIR="$(pwd)"
cd "${DIR_CURRENT}"

KIS_APPLOC="${DIR_CURRENT}/pkgWorkdir"


# -- Parse input args
for arg in "${@}"; do

    if [[ ${arg} = -f=* ]]; then
        DMG_INPUT_FILE="${arg#*=}"
        continue
    fi

    if [[ ${arg} = -e=* ]]; then
        KIS_ENTITLEMENTS="${arg#*=}"
        continue
    fi
    # If string starts with -sign
    if [[ ${arg} = -s=* ]]; then
        CODE_SIGNATURE="${arg#*=}"
        continue
    fi

    if [[ ${arg} = -name=* ]]; then
        DMG_NAME="${arg#*=}"
        continue
    fi

    if [[ ${arg} = -notarize-ac=* ]]; then
        NOTARIZE_ACC="${arg#*=}"
        continue
    fi

    if [[ ${arg} = -notarize-pass=* ]]; then
        NOTARIZE_PASS="${arg#*=}"
        continue
    fi


    if [[ ${arg} = -asc-provider=* ]]; then
        ASC_PROVIDER="${arg#*=}"
        continue
    fi

    if [[ ${arg} = "-h" || ${arg} = "--help" ]]; then
        print_usage
        exit 1
    fi
done

findEntitlementsFile

if [[ -z ${KIS_ENTITLEMENTS} ]]; then
    echo "Could not find entitlements file use for codesign"
    exit
else
    echo "using ${KIS_ENTITLEMENTS}"
fi

# -- Checks and messages

### Code Signature & NOTARIZATION
NOTARIZE="false"
if [[ -z "${CODE_SIGNATURE}" ]]; then
    echo "WARNING: No code signature provided, Code will not be signed"
else
    print_msg "Code will be signed with %s" "${CODE_SIGNATURE}"
    ### NOTARIZATION

    if [[ -n "${NOTARIZE_ACC}" ]]; then

        ASC_PROVIDER_OP=""
        if [[ -n "${ASC_PROVIDER}" ]]; then
            ASC_PROVIDER_OP="--asc-provider ${ASC_PROVIDER}"
        fi

        if [[ -z "${NOTARIZE_PASS}" ]]; then
            NOTARIZE_PASS="@keychain:AC_PASSWORD"
        fi

        # check if we can perform notarization
        xcrun altool --notarization-history 0 --username "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" ${ASC_PROVIDER_OP} 1> /dev/null

        if [[ ${?} -eq 0 ]]; then
            NOTARIZE="true"
        else
            echo "No password given for notarization or AC_PASSWORD missig in keychain"
        fi
    fi
fi

if [[ ${NOTARIZE} = "true" ]]; then
    print_msg "Signing checks complete, signatures are valid"
else
    echo "WARNING: Account information missing, Signtaure will not be performed"
    exit
fi


# Application is fetch from a .dmg or from .app
copyApplicationFolder

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
    printf "
use productbuild to prepare Pkg:

    \t productbuild --component ${KIS_APPLOC}/krita.app  /Applications krita5_submit.pkg --sign \"3rd Party Mac Developer Installer: ...\"

Then use altool to submit the Pkg:

    \t xcrun altool --upload-package krita5_submit.pkg --bundle-id <bundle-id> -t macos -u <appstore-username> --password <pass> [--asc-provider <teamid>] --bundle-version <version> --bundle-short-version-string <version> --apple-id <app-id_fromStore>

    \n"
fi

