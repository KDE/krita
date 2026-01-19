#!/usr/bin/env zsh
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# Krita tool to create dmg from installed source
# Copies all files to a folder to be converted into the final dmg

# osxdeploy.sh automates the creation of the release DMG.
#       default background and style are used if none provided

# A short explanation of what it does:

# - Copies krita.app contents to kritadmg folder
# - Copies i/share to Contents/Resources excluding unnecessary files
# - Copies translations, qml and quicklook PlugIns
# - Copies i/plugins and i/lib/plugins to Contents/PlugIns

# - Runs macdeployqt: macdeployqt is not built by default in ext_qt
#     build by:
#       cd ${BUILDROOT}/depbuild/ext_qt/ext_qt-prefix/src/ext_qt/qttools/src
#       make sub-macdeployqt-all
#       make sub-macdeployqt-install_subtargets
#       make install

#     the script changes dir to installation/bin to run macdeployqt as it can be buggy
#     if not run from the same folder as the binary is on.

# - Fix rpath from krita bin
# - Find missing libraries from plugins and copy to Frameworks or plugins.
#     This uses oTool iterative to find all unique libraries, then it searches each
#     library fond in <kritadmg> folder, and if not found attempts to copy contents
#     to the appropriate folder, either Frameworks (if frameworks is in namefile, or
#         library has plugin is not in path), or plugin if otherwise.

# - Builds DMG
#     Building DMG creates a new dmg with the contents of <kritadmg>
#     mounts the dmg and sets the style for dmg.
#     unmount
#     Compress resulting dmg into krita_nightly-<gitsha>.dmg
#     deletes temporary files.

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

# print status messages
print_msg() {
    printf "\e[32m${1}\e[0m\n" "${@:2}"
    # printf "%s\n" "${1}" >> ${OUTPUT_LOG}
}

# print error
print_error() {
    printf "\e[31m%s %s\e[0m\n" "Error:" "${1}"
}

# Debug helper function
debug_script() {
    echo "## running subcmd: ${@}"
    ${@}
}

if [[ -z ${DEBUG} ]]; then
    RUN_CMD="debug_script"
fi

DMG_title="krita" #if changed krita.temp.dmg must be deleted manually

# There is some duplication between build and deploy scripts
# a config env file could would be a nice idea.
if [[ -z "${KIS_SRC_DIR}" ]]; then
    KIS_SRC_DIR=${BUILDROOT}/
fi
if [[ -z "${KIS_BUILD_DIR}" ]]; then
    KIS_BUILD_DIR=${BUILDROOT}/_build
fi
KIS_INSTALL_DIR=${BUILDROOT}/_install
KRITA_DMG=${BUILDROOT}/_dmg
KRITA_DMG_TEMPLATE=${BUILDROOT}/_kritadmg-template

export PATH=${KIS_INSTALL_DIR}/bin:$PATH

# flags for OSX environment
echo "Trying to guess Qt version..."
if [[ -e "${KIS_INSTALL_DIR}/lib/QtCore.framework/Versions/A/QtCore" ]]; then
    echo " Found Qt6"
    # We only support from 12 up
    export MACOSX_DEPLOYMENT_TARGET=12
    export QMAKE_MACOSX_DEPLOYMENT_TARGET=12
else
    echo " Assuming Qt5"
    # We only support from 10.14 up
    export MACOSX_DEPLOYMENT_TARGET=10.14
    export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.14
fi

KRITA_VERSION="$(${KIS_INSTALL_DIR}/bin/krita_version -v)"

if [[ -n $KRITACI_RELEASE_PACKAGE_NAMING ]]; then
    # remove git-sha1, which is split with the space char
    KRITA_VERSION=${KRITA_VERSION% [a-f0-9]*}
else
    # merge version and git-sha1 for non-release versions
    KRITA_VERSION=${KRITA_VERSION// /-}
fi

print_usage () {
    printf "USAGE:
  osxdeploy.sh [-s=<identity>] [-notarize-ac=<apple-account>] [-style=<style.txt>] [-bg=<background-image>]

    -s \t\t\t Code sign identity for codesign

    -notarize-ac \t Apple account name for notarization purposes
\t\t\t script will attempt to get password from keychain, if fails provide one with
\t\t\t the -notarize-pass option: To add a password run

\t\t\t   security add-generic-password -a \"AC_USERNAME\" -w <secret_password> -s \"AC_PASSWORD\"

    -notarize-pass \t If given, the Apple account password. Otherwise an attempt will be macdeployqt_exists
\t\t\t to get the password from keychain using the account given in <notarize-ac> option.

    -asc-provider \t some AppleIds might need this option pass the <shortname>

    -style \t\t Style file defined from 'dmgstyle.sh' output

    -bg \t\t Set a background image for dmg folder.

    -name \t\t Set the DMG name output.

\t\t\t osxdeploy needs an input image to attach to the dmg background
\t\t\t image recommended size is at least 950x500
"
}

# Attempt to detach previous mounted DMG
if [[ -d "/Volumes/${DMG_title}" ]]; then
    echo "WARNING: Another Krita DMG is mounted!"
    echo "Attempting eject…"
    hdiutil detach "/Volumes/${DMG_title}"
    if [ $? -ne 0  ]; then
        exit 1
    fi
    echo "Success!"
fi

# -- Parse input args
for arg in "${@}"; do
    if [ "${arg}" = -bg=* -a -f "${arg#*=}" ]; then
        DMG_validBG=0
        bg_filename=${arg#*=}
        echo "attempting to check background is valid jpg or png..."
        BG_FORMAT=$(sips --getProperty format ${bg_filename} | awk '{printf $2}')

        if [[ "png" = ${BG_FORMAT} || "jpeg" = ${BG_FORMAT} ]];then
            echo "valid image file"
            DMG_background=$(cd "$(dirname "${bg_filename}")"; pwd -P)/$(basename "${bg_filename}")
            DMG_validBG=1
            # check imageDPI
            BG_DPI=$(sips --getProperty dpiWidth ${DMG_background} | grep dpi | awk '{print $2}')
            if [[ $(echo "${BG_DPI} > 150" | bc -l) -eq 1 ]]; then
            printf "WARNING: image dpi has an effect on apparent size!
    Check dpi is adequate for screen display if image appears very small
    Current dpi is: %s\n" ${BG_DPI}
            fi
        fi
    fi
    # If string starts with -sign
    if [[ ${arg} = -s=* ]]; then
        CODE_SIGNATURE="${arg#*=}"
    fi

    if [[ ${arg} = -name=* ]]; then
        DMG_NAME="${arg#*=}"
    fi

    if [[ ${arg} = -notarize-ac=* ]]; then
        NOTARIZE_ACC="${arg#*=}"
    fi

    if [[ ${arg} = -notarize-pass=* ]]; then
        NOTARIZE_PASS="${arg#*=}"
    fi

    if [[ ${arg} = -style=* ]]; then
        style_filename="${arg#*=}"
        if [[ -f "${style_filename}" ]]; then
            DMG_STYLE="${style_filename}"
        fi
    fi

    if [[ ${arg} = -asc-provider=* ]]; then
        APPLE_TEAMID="${arg#*=}"
    fi

    if [[ ${arg} = "-h" || ${arg} = "--help" ]]; then
        print_usage
        exit 1
    fi
done

# -- Checks and messages

### PYTHONAttempt to find python_version
local_PY_MAYOR_VERSION=$(python -c "import sys; print(sys.version_info[0])")
local_PY_MINOR_VERSION=$(python -c "import sys; print(sys.version_info[1])")
PY_VERSION="${local_PY_MAYOR_VERSION}.${local_PY_MINOR_VERSION}"

print_msg "Detected Python %s" "${PY_VERSION}"

### Code Signature & NOTARIZATION
NOTARIZE="false"
if [[ -z "${CODE_SIGNATURE}" ]]; then
    echo "WARNING: No code signature provided, Code will not be signed"
else
    print_msg "Code will be signed with %s" "${CODE_SIGNATURE}"
    ### NOTARIZATION
    # check if we can perform notarization using notarytool
    xcrun notarytool history --keychain-profile KritaNotarizeAccount 1> /dev/null
    if [[ ${?} -eq 0 ]]; then
        NOTARYTOOL="short"
        NOTARIZE="true"
    fi

    if [[ ${NOTARIZE} = "false" && -n "${NOTARIZE_ACC}" ]]; then
        NOTARIZE="true"
        ASC_PROVIDER_OP=""

        if [[ -z ${APPLE_TEAMID} ]]; then
            echo "No team id provided, extracting from signature"
            APPLE_TEAMID=${CODE_SIGNATURE[-11,-2]}
        fi

        if [[ -n "${APPLE_TEAMID}" ]]; then
            ASC_PROVIDER_OP="--asc-provider ${APPLE_TEAMID}"
        fi

        if [[ -z "${NOTARIZE_PASS}" ]]; then
            NOTARIZE_PASS="@keychain:AC_PASSWORD"
            KEYCHAIN_PASS="true"
        fi

        # check if we can perform notarization
        
        xcrun notarytool history --apple-id "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" --team-id "${APPLE_TEAMID}" 1> /dev/null
        if [[ ${?} -ne 0 ]]; then
            echo "Unable to use notarytool: not setup/missing password, trying altool"
            ALTOOL="true"
            xcrun altool --notarization-history 0 --username "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" ${ASC_PROVIDER_OP} 1> /dev/null

            if [[ ${?} -ne 0 ]]; then
                NOTARIZE="false"
                echo "No password given for notarization or AC_PASSWORD missing in keychain"
            fi
        else
            NOTARYTOOL="long"
        fi
    fi
fi

if [[ ${NOTARIZE} = "true" ]]; then
    print_msg "Notarization checks complete, This build will be notarized"
else
    echo "WARNING: Account information missing, Notarization will not be performed"
fi


### STYLE for DMG
if [[ ! ${DMG_STYLE} ]]; then
    DMG_STYLE="${KIS_SRC_DIR}/packaging/macos/default.style"
fi

print_msg "Using style from: %s" "${DMG_STYLE}"

### Background for DMG
if [[ ${DMG_validBG} -eq 0 ]]; then
    echo "No jpg or png valid file detected!!"
    echo "Using default style"
    DMG_background="${KIS_SRC_DIR}/packaging/macos/krita_dmgBG.png"
fi



# Helper functions
countArgs () {
    echo "${#}"
}

stringContains () {
    echo "$(grep "${2}" <<< "${1}")"
}

waiting_fixed() {
    local message="${1}"
    local waitTime=${2}

    for i in $(seq ${waitTime}); do
        sleep 1
        printf -v dots '%*s' ${i}
        printf -v spaces '%*s' $((${waitTime} - $i))
        printf "\r%s [%s%s]" "${message}" "${dots// /.}" "${spaces}"
    done
    printf "\n"
}

add_lib_to_list() {
    local llist=${2}
    local libPathTail="${1##*/}"
    local lib=${libPathTail}

    # we add a special case if the lib is a framework
    if [[ -n "$(grep ${libPathTail}.framework <<< ${1})" ]]; then
        lib="${libPathTail}.framework"
    fi

    if [[ -z "$(grep ${lib} <<< ${llist})" ]]; then
        local llist="${llist} ${lib} "
    fi
    echo "${llist}"
}

# Find all @rpath and Absolute to buildroot path libs
# Add to libs_used
# converts absolute buildroot path to @rpath
find_needed_libs () {
    # echo "Analyzing libraries with oTool..." >&2
    local libs_used="" # input lib_lists founded

    for libFile in ${@}; do
        if [[ -z "$(file ${libFile} | grep 'Mach-O')" ]]; then
            # echo "skipping ${libFile}" >&2
            continue
        fi

        resultArray=($(otool -L ${libFile} | awk '{print $1","substr($2,2)}'))

        for entry in ${resultArray[@]:1}; do
            # skip fat-bin file markers
            if [[ "${entry##*,}" = "architecture" ]]; then
                continue
            fi

            lib="${entry%%,*}"
            printf "checking %s\n" "${lib}" >&2

            if [[ "${lib:0:1}" = "@" ]]; then
                printf "Fixing %s from %s\n" "${lib}" "${libFile}" >&2
                local libs_used=$(add_lib_to_list "${lib}" "${libs_used}")
            fi

            if [[ "${lib:0:${#BUILDROOT}}" = "${BUILDROOT}" ]]; then
                printf "\t%s\n" "${lib}" >&2
                if [[ "${lib##*/}" = "${libFile##*/}" ]]; then
                    install_name_tool -id ${lib##*/} "${libFile}"
                else
                    install_name_tool -change ${lib} "@rpath/${lib##*/i/lib/}" "${libFile}"
                    local libs_used=$(add_lib_to_list "${lib}" "${libs_used}")
                fi
            fi
        done
    done
    echo "${libs_used}" # return updated list
}

find_missing_libs (){
    # echo "Searching for missing libs on deployment folders…" >&2
    local libs_missing=""
    for lib in ${@}; do
        printf "looking for %s\n" "${lib}" >&2
        if [[ -z "$(find ${KRITA_DMG}/krita.app/Contents/ -name ${lib})" ]]; then
            echo "Adding ${lib} to missing libraries." >&2
            libs_missing="${libs_missing} ${lib}"
        fi
    done
    echo "${libs_missing}"
}

copy_missing_lib_wlink() {
    local inlibfile=${1}
    local dst=${2}

    local sourceLibDir=$(dirname ${inlibfile})
    local libfile=$(basename ${inlibfile})

    while [[ true ]]; do
        local lib="${sourceLibDir}/${libfile}"
        ${RUN_CMD} cp -av ${lib} ${dst}
        libfile=$(readlink ${lib})
        if [[ -z ${libfile} ]]; then break; fi
    done
}

copy_missing_libs () {
    for lib in ${@}; do
        result=$(find -L "${KIS_INSTALL_DIR}" -name "${lib}")

        # check if missing lib is a framework
        if [[ ${lib} = ${lib%.framework} ]]; then
            if [ "$(stringContains "${result}" "plugin")" ]; then
                copy_missing_lib_wlink ${result} ${KRITA_DMG}/krita.app/Contents/PlugIns/
                krita_findmissinglibs "${KRITA_DMG}/krita.app/Contents/PlugIns/${result##*/}"
            else
                copy_missing_lib_wlink ${result} ${KRITA_DMG}/krita.app/Contents/Frameworks/
                krita_findmissinglibs "${KRITA_DMG}/krita.app/Contents/Frameworks/${result##*/}"
            fi
        else
            echo "copying missing framework ${lib}"
            rsync -priul ${KIS_INSTALL_DIR}/lib/${lib} ${KRITA_DMG}/krita.app/Contents/Frameworks/
            krita_findmissinglibs "$(find "${KRITA_DMG}/krita.app/Contents/Frameworks/${lib}/" -type f -perm 755)"
        fi
    done
}

krita_findmissinglibs() {
    neededLibs=$(find_needed_libs ${@})
    missingLibs=$(find_missing_libs ${neededLibs})

    if [[ $(countArgs ${missingLibs}) -gt 0 ]]; then
        printf "Found missing libs: %s\n" "${missingLibs}"
        copy_missing_libs ${missingLibs}
    fi
}

strip_python_dmginstall() {
    # reduce size of framework python
    # Removes tests, installers, pyenv, distutils
    echo "Removing unnecessary files from Python.Framework to be packaged..."
    PythonFrameworkBase="${KRITA_DMG}/krita.app/Contents/Frameworks/Python.framework"

    cd "${PythonFrameworkBase}"
    find . -name "test*" -type d | xargs rm -rf
    find "${PythonFrameworkBase}/Versions/${PY_VERSION}/bin" -not -name "python*" \( -type f -or -type l \) | xargs rm -f
    cd "${PythonFrameworkBase}/Versions/${PY_VERSION}/lib/python${PY_VERSION}"
    rm -rf distutils tkinter ensurepip venv lib2to3 idlelib turtledemo
    # remove tkinter module
    rm "./lib-dynload/_tkinter.cpython-${local_PY_MAYOR_VERSION}${local_PY_MINOR_VERSION}-darwin.so"

    cd "${PythonFrameworkBase}/Versions/${PY_VERSION}/lib/python${PY_VERSION}/site-packages"
    rm -rf pip* PyQt_builder* setuptools* sip* easy-install.pth

    cd "${PythonFrameworkBase}/Versions/${PY_VERSION}/Resources"
    rm -rf Python.app
}

# Remove any missing rpath pointing to BUILDROOT
libs_clean_rpath () {
    for libFile in ${@}; do
        rpath=($(otool -l "${libFile}" | grep "path ${BUILDROOT}" | awk '{$1=$1;print $2}'))
        for lpath in "${rpath[@]}"; do
            echo "removed rpath _${lpath}_ from ${libFile}"
            install_name_tool -delete_rpath "${lpath}" "${libFile}" 2> /dev/null
        done
    done
}

# Multithreaded version
# of libs_clean_rpath, but makes assumptions
delete_install_rpath() {
    xargs -P4 -I FILE install_name_tool -delete_rpath "${KIS_INSTALL_DIR}/lib" FILE 2> "${BUILDROOT}/deploy_error.log"
}

fix_python_framework() {
    # Fix python.framework rpath and slims down installation
    PythonFrameworkBase="${KRITA_DMG}/krita.app/Contents/Frameworks/Python.framework"

    # Fix permissions
    find "${PythonFrameworkBase}" -name "*.so" | xargs -P4 -I FILE chmod a+x FILE 2> "${BUILDROOT}/deploy_error.log"
    cd "${PythonFrameworkBase}/Versions/${PY_VERSION}/lib/python${PY_VERSION}"
    chmod a+x pydoc.py

    # Fix main library
    pythonLib="${PythonFrameworkBase}/Python"
    install_name_tool -id "${pythonLib##*/}" "${pythonLib}"
    install_name_tool -add_rpath @loader_path/../../../ "${pythonLib}" 2> /dev/null
    install_name_tool -change @loader_path/../../../../libintl.9.dylib @loader_path/../../../libintl.9.dylib "${pythonLib}"

    # Fix all executables
    install_name_tool -add_rpath @executable_path/../../../../../../../ "${PythonFrameworkBase}/Versions/Current/Resources/Python.app/Contents/MacOS/Python"
    install_name_tool -change "${KIS_INSTALL_DIR}/lib/Python.framework/Versions/${PY_VERSION}/Python" @executable_path/../../../../../../Python "${PythonFrameworkBase}/Versions/Current/Resources/Python.app/Contents/MacOS/Python"
    install_name_tool -add_rpath @executable_path/../../../../ "${PythonFrameworkBase}/Versions/Current/bin/python${PY_VERSION}"

    # Fix rpaths from Python.Framework
    find "${PythonFrameworkBase}" -type f -perm 755 | delete_install_rpath
    find "${PythonFrameworkBase}/Versions/Current/lib/python${PY_VERSION}/site-packages/PyQt5" -type f -name "*.so" | delete_install_rpath
}

plistbuddy_add() {
    local plistbuddy="/usr/libexec/PlistBuddy"
    local filename="${1}"
    local key="${2}"
    local type="${3}"
    local value="${4}"

    ${plistbuddy} "${filename}" -c "Add:${key} ${type} ${value}"
}

krita_create_PyKrita() {
    local plistbuddy="/usr/libexec/PlistBuddy"
    local fmwk_name="PyKrita"
    local fmwk_version=${KRITA_VERSION%%-*}

    # assume we are not in frameworks
    local old_pwd=$(pwd)
    cd ${KRITA_DMG}/krita.app/Contents/Frameworks/

    mkdir ${fmwk_name}.framework
    mkdir ${fmwk_name}.framework/Versions

    cd ${fmwk_name}.framework/Versions
    mkdir ${fmwk_version}
    ln -s ${fmwk_version} Current

    cd ${fmwk_version}
    mkdir Resources lib

    rsync -priul ${KIS_INSTALL_DIR}/lib/krita-python-libs/ lib

    mv lib/PyKrita/krita.so ${fmwk_name}
    ln -s ../../${fmwk_name} lib/PyKrita/krita.so

    cd Resources
    plistbuddy_add Info.plist CFBundleExecutable string ${fmwk_name}
    plistbuddy_add Info.plist CFBundleIdentifier string org.krita.${fmwk_name}
    plistbuddy_add Info.plist CFBundlePackageType string FMWK
    plistbuddy_add Info.plist CFBundleShortVersionString string ${fmwk_version}
    plistbuddy_add Info.plist CFBundleVersion string ${fmwk_version}

    cd ../../..
    ln -s Versions/Current/${fmwk_name} ${fmwk_name}
    ln -s Versions/Current/Resources Resources

    cd ../
    ln -s ./${fmwk_name}.framework/Versions/Current/lib/ krita-python-libs

    cd ${old_pwd}
}

# Checks for macdeployqt
# If not present attempts to install
# If it fails shows an informative message
# (For now, macdeployqt is fundamental to deploy)
macdeployqt_exists() {
    printf "Checking for macdeployqt...  "

    if [[ ! -e "${KIS_INSTALL_DIR}/bin/macdeployqt" ]]; then
        printf "Not Found!\n"
        printf "Attempting to install macdeployqt\n"

        cd "${BUILDROOT}/depbuild/ext_qt/ext_qt-prefix/src/ext_qt/qttools/src"
        make sub-macdeployqt-all
        make sub-macdeployqt-install_subtargets
        make install

        if [[ ! -e "${KIS_INSTALL_DIR}/bin/macdeployqt" ]]; then
        printf "
ERROR: Failed to install macdeployqt!

    Compile and install from qt source directory
    Source code to build could be located in qttools/src in qt source dir:

        ${BUILDROOT}/depbuild/ext_qt/ext_qt-prefix/src/ext_qt/qttools/src

    From the source dir, build and install:

        make sub-macdeployqt-all
        make sub-macdeployqt-install_subtargets
        make install
"
        printf "\nexiting...\n"
        exit 1
        else
            echo "Done!"
        fi
    else
        echo "Found!"
    fi
}

run_macdeployqt() {
    # To avoid errors macdeployqt must be run from bin location
    # ext_qt will not build macdeployqt by default so it must be build manually
    #   cd ${BUILDROOT}/depbuild/ext_qt/ext_qt-prefix/src/ext_qt/qttools/src
    #   make sub-macdeployqt-all
    #   make sub-macdeployqt-install_subtargets
    #   make install
    echo "Running macdeployqt..."
    cd ${KIS_INSTALL_DIR}/bin
    ./macdeployqt ${KRITA_DMG}/krita.app \
        -verbose=0 \
        -executable=${KRITA_DMG}/krita.app/Contents/MacOS/krita \
        -libpath=${KIS_INSTALL_DIR}/lib \
        -qmldir=${KIS_SRC_DIR}/plugins/dockers/textproperties \
        -appstore-compliant
        # -extra-plugins=${KIS_INSTALL_DIR}/lib/kritaplugins \
        # -extra-plugins=${KIS_INSTALL_DIR}/lib/plugins \
        # -extra-plugins=${KIS_INSTALL_DIR}/plugins

    cd ${BUILDROOT}
    echo "macdeployqt done!"
}

krita_deploy () {
    # check for macdeployqt
    macdeployqt_exists

    cd ${BUILDROOT}
    # Update files in krita.app
    echo "Deleting previous kritadmg run..."
    rm -rf ./krita.dmg ${KRITA_DMG}
    # Copy new builtFiles
    echo "Preparing ${KRITA_DMG} for deployment..."

    echo "Copying krita.app..."
    mkdir "${KRITA_DMG}"

    rsync -prul ${KIS_INSTALL_DIR}/bin/krita.app ${KRITA_DMG}
    cp ${KIS_INSTALL_DIR}/bin/kritarunner ${KRITA_DMG}/krita.app/Contents/MacOS
    cp ${KIS_INSTALL_DIR}/bin/krita_version ${KRITA_DMG}/krita.app/Contents/MacOS

    mkdir -p ${KRITA_DMG}/krita.app/Contents/PlugIns
    mkdir -p ${KRITA_DMG}/krita.app/Contents/Frameworks

    echo "Copying share..."
    # Deletes old copies of translation and qml to be recreated
    cd ${KIS_INSTALL_DIR}/share/
    rsync -prul --delete ./ \
            --exclude krita.icns \
            --exclude krita-krz.icns \
            --exclude krita-kra.icns \
            --exclude Assets.car \
            --exclude aclocal \
            --exclude doc \
            --exclude ECM \
            --exclude eigen3 \
            --exclude emacs \
            --exclude gettext \
            --exclude gettext-0.19.8 \
            --exclude info \
            --exclude kf5 \
            --exclude kservices5 \
            --exclude man \
            --exclude ocio \
            --exclude pkgconfig \
            --exclude mime \
            --exclude translations \
            --exclude qml \
            ${KRITA_DMG}/krita.app/Contents/Resources


    cd ${BUILDROOT}

    echo "Copying Qt translations..."
    rsync -prul ${KIS_INSTALL_DIR}/translations ${KRITA_DMG}/krita.app/Contents/

    cd ${KRITA_DMG}/krita.app/Contents
    ln -shF Resources share
    ln -shF Frameworks lib

    echo "Copying mandatory libs..."
    rsync -priul ${KIS_INSTALL_DIR}/lib/libKF5* ${KIS_INSTALL_DIR}/lib/libkrita* Frameworks/

    echo "Copying plugins..."
    local KRITA_DMG_PLUGIN_DIR="${KRITA_DMG}/krita.app/Contents/PlugIns"
    
    # do not copy excluded files but delete if present in dst
    cd ${KIS_INSTALL_DIR}/plugins/
    rsync -prul --delete --delete-excluded ./ \
        --exclude kritaquicklook.qlgenerator \
        --exclude kritaspotlight.mdimporter \
        ${KRITA_DMG_PLUGIN_DIR}
    
    echo "Copying QuickLook plugin..."
    mkdir -p ${KRITA_DMG}/krita.app/Contents/Library/QuickLook
    rsync -prul ${KIS_INSTALL_DIR}/plugins/kritaquicklook.qlgenerator ${KRITA_DMG}/krita.app/Contents/Library/QuickLook
    echo "Copying Spotlight plugin..."
    mkdir -p ${KRITA_DMG}/krita.app/Contents/Library/Spotlight
    rsync -prul ${KIS_INSTALL_DIR}/plugins/kritaspotlight.mdimporter ${KRITA_DMG}/krita.app/Contents/Library/Spotlight

    # thumbnails for macOS 11.0 and up
    echo "Copying Krita Thumbnailing extension..."
    rsync -prul ${KIS_INSTALL_DIR}/plugins/krita-thumbnailer.appex ${KRITA_DMG_PLUGIN_DIR}

    echo "Copying Krita Preview extension..."
    rsync -prul ${KIS_INSTALL_DIR}/plugins/krita-preview.appex ${KRITA_DMG_PLUGIN_DIR}


    cd ${BUILDROOT}
    rsync -prul ${KIS_INSTALL_DIR}/lib/kritaplugins/* ${KRITA_DMG_PLUGIN_DIR}

    rsync -prul ${KIS_INSTALL_DIR}/lib/mlt ${KRITA_DMG_PLUGIN_DIR}
    
    echo "Adding ffmpeg to bundle"
    cd ${KRITA_DMG}/krita.app/Contents/MacOS
    for bin in ffmpeg ffprobe; do
        cp ${KIS_INSTALL_DIR}/bin/${bin} ./
        install_name_tool -add_rpath @executable_path/../Frameworks/ ${bin}
    done

    # rsync -prul {KIS_INSTALL_DIR}/lib/libkrita* Frameworks/

    echo "Copying python..."
    # Copy this framework last!
    # It is best that macdeployqt does not modify Python.framework
    # folders with period in name are treated as Frameworks for codesign
    rsync -prul ${KIS_INSTALL_DIR}/lib/Python.framework ${KRITA_DMG}/krita.app/Contents/Frameworks/
    krita_create_PyKrita

    # change perms on Python to allow header change
    chmod +w ${KRITA_DMG}/krita.app/Contents/Frameworks/Python.framework/Python

    fix_python_framework
    strip_python_dmginstall

    # fix python pyc
    # precompile all pyc so the dont alter signature
    echo "Precompiling all python files..."
    cd ${KRITA_DMG}/krita.app
    ${KIS_INSTALL_DIR}/bin/python -m compileall . &> /dev/null

    # remove unnecessary rpaths
    install_name_tool -delete_rpath @executable_path/../lib ${KRITA_DMG}/krita.app/Contents/MacOS/krita_version
    install_name_tool -delete_rpath @executable_path/../lib ${KRITA_DMG}/krita.app/Contents/MacOS/kritarunner
    install_name_tool -delete_rpath @loader_path/../../../../lib ${KRITA_DMG}/krita.app/Contents/MacOS/krita
    rm -rf ${KRITA_DMG}/krita.app/Contents/PlugIns/kf5/org.kde.kwindowsystem.platforms

    # Fix permissions
    find "${KRITA_DMG}/krita.app/Contents" -type f -name "*.dylib" -or -name "*.so" -or -path "*/Contents/MacOS/*" | xargs -P4 -I FILE chmod a+x FILE
    find "${KRITA_DMG}/krita.app/Contents/Resources/applications" -name "*.desktop" | xargs -P4 -I FILE chmod a-x FILE

    # repair krita for plugins
    printf "Searching for missing libraries\n"
    krita_findmissinglibs $(find ${KRITA_DMG}/krita.app/Contents -type f -perm 755 -or -name "*.dylib" -or -name "*.so")

    # we delay macdeployqt after searching for libs as it does not follow links correctly
    run_macdeployqt

    # Fix rpath for plugins
    # Uncomment if the Finder plugins (kritaquicklook, kritaspotlight) lack the rpath below
    printf "Repairing rpath for Finder plugins\n"
    find "${KRITA_DMG}/krita.app/Contents/Library" -type f -path "*/Contents/MacOS/*" -perm 755 | xargs -I FILE install_name_tool -add_rpath @loader_path/../../../../../Frameworks FILE

    printf "removing absolute or broken linksys, if any\n"
    find "${KRITA_DMG}/krita.app/Contents" -type l \( -lname "/*" -or -not -exec test -e {} \; \) -print | xargs rm -v

    printf "clean any left over rpath\n"
    libs_clean_rpath $(find "${KRITA_DMG}/krita.app/Contents" -type f -perm 755 -or -name "*.dylib" -or -name "*.so")
#    libs_clean_rpath $(find "${KRITA_DMG}/krita.app/Contents/" -type f -name "lib*")

    # remove debug version as both versions can't be signed.
    rm ${KRITA_DMG}/krita.app/Contents/Frameworks/QtScript.framework/Versions/Current/QtScript_debug
    echo "## Finished preparing krita.app bundle!"

}


# helper to define function only once
batch_codesign() {
    local entitlements="${1}"
    if [[ -z "${1}" ]]; then
        entitlements="${KIS_SRC_DIR}/packaging/macos/entitlements.plist"
    fi
    xargs -P4 -I FILE codesign --options runtime --timestamp -f -s "${CODE_SIGNATURE}" --entitlements "${entitlements}" FILE
}

# Code sign must be done as recommended by apple "sign code inside out in individual stages"
signBundle() {
    # sign Frameworks and libs
    cd ${KRITA_DMG}/krita.app/Contents/Frameworks

    # Do not sign binaries inside frameworks except for Python's
    find . -type d -path "*.framework" -prune -false -o -perm +111 -not -type d | batch_codesign
    find Python.framework -type f -name "*.o" -or -name "*.so" -or -perm +111 -not -type d -not -type l | batch_codesign
    find . -type d -name "*.framework" | xargs printf "%s/Versions/Current\n" | batch_codesign

    # Sign all other files in Framework (needed)
    # there are many files in python do we need to sign them all?
    find krita-python-libs -type f | batch_codesign
    # find python -type f | batch_codesign

    # Sign only libraries and plugins
    cd ${KRITA_DMG}/krita.app/Contents/PlugIns
    find . -type f | batch_codesign

    cd ${KRITA_DMG}/krita.app/Contents/Library/QuickLook
    printf "kritaquicklook.qlgenerator" | batch_codesign

    cd ${KRITA_DMG}/krita.app/Contents/Library/Spotlight
    printf "kritaspotlight.mdimporter" | batch_codesign

    cd ${KRITA_DMG}/krita.app/Contents/PlugIns
    printf "krita-thumbnailer.appex" | batch_codesign
    printf "krita-preview.appex" | batch_codesign

    # It is necessary to sign every binary Resource file
    cd ${KRITA_DMG}/krita.app/Contents/Resources
    find . -perm +111 -type f | batch_codesign

    printf "${KRITA_DMG}/krita.app/Contents/MacOS/ffmpeg" | batch_codesign
    printf "${KRITA_DMG}/krita.app/Contents/MacOS/ffprobe" | batch_codesign
    
    printf "${KRITA_DMG}/krita.app/Contents/MacOS/kritarunner" | batch_codesign
    printf "${KRITA_DMG}/krita.app/Contents/MacOS/krita_version" | batch_codesign

    #Finally sign krita and krita.app
    printf "${KRITA_DMG}/krita.app/Contents/MacOS/krita" | batch_codesign
    printf "${KRITA_DMG}/krita.app" | batch_codesign
}

sign_hasError() {
    local CODESIGN_STATUS=0
    for f in $(find "${KRITA_DMG}" -type f); do
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

# Notarize build on macOS servers
# based on https://github.com/Beep6581/RawTherapee/blob/6fa533c40b34dec527f1176d47cc6c683422a73f/tools/osx/macosx_bundle.sh#L225-L250
notarize_build() {
    local NOT_SRC_DIR=${1}
    local NOT_SRC_FILE=${2}

    local notarization_complete="true"

    if [[ ${NOTARIZE} = "true" ]]; then
        printf "performing notarization of %s\n" "${2}"
        cd "${NOT_SRC_DIR}"

        ditto -c -k --sequesterRsrc --keepParent "${NOT_SRC_FILE}" "${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip"

        if [[ ${NOTARYTOOL} = "short" ]]; then
            xcrun notarytool submit "${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip" --wait --keychain-profile KritaNotarizeAccount

        elif [[ ${NOTARYTOOL} = "long" ]]; then
            xcrun notarytool submit "${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip" --wait --apple-id "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" --team-id "${APPLE_TEAMID}"

        else
            # echo "xcrun altool --notarize-app --primary-bundle-id \"org.krita\" --username \"${NOTARIZE_ACC}\" --password \"${NOTARIZE_PASS}\" --file \"${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip\""
            local altoolResponse="$(xcrun altool --notarize-app --primary-bundle-id "org.krita" --username "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" ${ASC_PROVIDER_OP} --file "${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip" 2>&1)"

            if [[ -n "$(grep 'Error' <<< ${altoolResponse})" ]]; then
                printf "ERROR: xcrun altool exited with the following error! \n\n%s\n\n" "${altoolResponse}"
                printf "This could mean there is an error in AppleID authentication!\n"
                printf "aborting notarization\n"
                NOTARIZE="false"
                return
            else
                printf "Response:\n\n%s\n\n" "${altoolResponse}"
            fi

            local uuid="$(grep 'RequestUUID' <<< ${altoolResponse} | awk '{ print $NF }')"
            echo "RequestUUID = ${uuid}" # Display identifier string

            waiting_fixed "Waiting to retrieve notarize status" 120

            while true ; do
                fullstatus=$(xcrun altool --notarization-info "${uuid}" --username "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" ${ASC_PROVIDER_OP} 2>&1)  # get the status
                notarize_status=`echo "${fullstatus}" | grep 'Status\:' | awk '{ print $2 }'`
                echo "${fullstatus}"
                if [[ "${notarize_status}" = "success" ]]; then
                    print_msg "Notarization success!"
                    break
                elif [[ "${notarize_status}" = "in" ]]; then
                    waiting_fixed "Notarization still in progress, wait before checking again" 60
                else
                    notarization_complete="false"
                    echo "Notarization failed! full status below"
                    echo "${fullstatus}"
                    exit 1
                fi
            done
        fi

        if [[ "${notarization_complete}" = "true" ]]; then
            xcrun stapler staple "${NOT_SRC_FILE}"   #staple the ticket
            xcrun stapler validate -v "${NOT_SRC_FILE}"
        fi
    fi
}

createDMG () {
    printf "Creating of dmg with contents of %s...\n" "${KRITA_DMG}"
    cd ${BUILDROOT}
    DMG_size=1500

    if [[ -z "${DMG_NAME}" ]]; then
        # Add git version number
        DMG_NAME="krita-${KRITA_VERSION}.dmg"
    else
        DMG_NAME="${DMG_NAME}.dmg"
    fi

    ## Build dmg from folder

    # create dmg on local system
    # usage of -fsargs minimize gaps at front of filesystem (reduce size)
    hdiutil create -srcfolder "${KRITA_DMG}" -volname "${DMG_title}" -fs APFS \
        -format UDIF -verbose -size ${DMG_size}m krita.temp.dmg

    # Next line is only useful if we have a dmg as a template!
    # previous hdiutil must be uncommented
    # cp krita-template.dmg krita.dmg
    device=$(hdiutil attach -readwrite -noverify -noautoopen "krita.temp.dmg" | egrep '^/dev/' | sed 1q | awk '{print $1}')

    # rsync -priul --delete ${KRITA_DMG}/krita.app "/Volumes/${DMG_title}"

    # Set style for dmg
    if [[ ! -d "/Volumes/${DMG_title}/.background" ]]; then
        mkdir "/Volumes/${DMG_title}/.background"
    fi
    cp -v ${DMG_background} "/Volumes/${DMG_title}/.background/"

    mkdir "/Volumes/${DMG_title}/Terms of Use"
    cp -v "${KIS_SRC_DIR}/packaging/macos/Terms_of_use.rtf" "/Volumes/${DMG_title}/Terms of Use/"
    ln -s "/Applications" "/Volumes/${DMG_title}/Applications"

    #Set Icon for DMG
    cp -v "${KIS_SRC_DIR}/packaging/macos/KritaIcon.icns" "/Volumes/${DMG_title}/.VolumeIcon.icns"
    SetFile -a C "/Volumes/${DMG_title}"

    ## Apple script to set style
    style="$(<"${DMG_STYLE}")"
    printf "${style}" "${DMG_title}" "${DMG_background##*/}" | osascript

    chmod -Rf go-w "/Volumes/${DMG_title}"
    # ensure all writing operations to dmg are over
    sync

    hdiutil detach $device
    hdiutil convert "krita.temp.dmg" -format UDZO -imagekey -zlib-level=9 -o krita-out.dmg


    mv krita-out.dmg ${DMG_NAME}
    echo "moved krita-out.dmg to ${DMG_NAME}"
    rm krita.temp.dmg

    if [[ -n "${CODE_SIGNATURE}" ]]; then
        printf "${DMG_NAME}" | batch_codesign
    fi

    # notarize_build "${BUILDROOT}" "${DMG_NAME}"

    if [[ ! -d "${BUILDROOT}/_packaging" ]]; then
        mkdir "${BUILDROOT}/_packaging"
    fi

    mv ${DMG_NAME} ${BUILDROOT}/_packaging/

    echo "dmg done!"
}

#######################
# Program starts!!
########################
# Run deploy command, installation is assumed to exist in BUILDROOT/i
if [[ -z "${ONLY_DMG}" ]]; then
    krita_deploy
fi

# # Code sign krita.app if signature given
# if [[ -n "${CODE_SIGNATURE}" ]]; then
#     signBundle
# fi

# # Manually check every single Mach-O file for signature status
# if [[ "${NOTARIZE}" = "true" ]]; then
#     print_msg "Checking if all files are signed before sending for notarization..."
#     if [[ $(sign_hasError) -eq 1 ]]; then
#         print_error "CodeSign errors cannot send to notarize!"
#         echo "krita.app not sent to notarization, stopping...."
#         exit 1
#     fi
#     print_msg "Done! all files appear to be correct."

#     # notarize apple
#     notarize_build "${KRITA_DMG}" krita.app
# fi

# Create DMG from files inside ${KRITA_DMG} folder
if [[ -z "${ONLY_APP}" ]]; then
    createDMG
fi

# if [[ "${NOTARIZE}" = "false" ]]; then
#     macosVersion="$(sw_vers | grep ProductVersion | awk '
#        BEGIN { FS = "[ .\t]" }
#              { print $3}
#     ')"
#     if (( ${macosVersion} == 15 )); then
#         print_error "Build not notarized! Needed for macOS versions above 10.14"
#     fi
# fi
