#!/usr/bin/env bash

# Krita tool to create dmg from installed source
# Copies all files to a folder to be converted into the final dmg

# osxdeploy.sh automates the creation of the release DMG.
#       default background and style are used if none provided

# A short explanation of what it does:

# - Copies krita.app contents to kritadmg folder
# - Copies i/share to Contents/Resources excluding unnecesary files
# - Copies translations, qml and quicklook PlugIns
# - Copies i/plugins and i/lib/plugins to Contents/PlugIns

# - Runs macdeployqt: macdeployqt is not built by default in ext_qt
#     build by:
#       cd ${BUILDROOT}/depbuild/ext_qt/ext_qt-prefix/src/ext_qt/qttools/src
#       make sub-macdeployqt-all
#       make sub-macdeployqt-install_subtargets
#       make install

#     the script changes dir to installation/bin to run macdeployqt as it can be buggy
#     if not runned from the same folder as the binary is on.

# - Fix rpath from krita bin
# - Find missing libraries from plugins and copy to Framworks or plugins.
#     This uses oTool iterative to find all unique libraries, then it searches each
#     library fond in <kritadmg> folder, and if not found attempts to copy contents
#     to the appropiate folder, either Frameworks (if frameworks is in namefile, or
#         library has plugin isnot in path), or plugin if otherwise.

# - Builds DMG
#     Building DMG creates a new dmg with the contents of <kritadmg>
#     mounts the dmg and sets the style for dmg.
#     unmount
#     Compress resulting dmg into krita_nightly-<gitsha>.dmg
#     deletes temporary files.

if test -z ${BUILDROOT}; then
    echo "ERROR: BUILDROOT env not set!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    echo "exiting..."
    exit
fi

# print status messges
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

DMG_title="krita" #if changed krita.temp.dmg must be deleted manually
SCRIPT_SOURCE_DIR="$(get_script_dir)"

# There is some duplication between build and deploy scripts
# a config env file could would be a nice idea.
KIS_SRC_DIR=${BUILDROOT}/krita
KIS_INSTALL_DIR=${BUILDROOT}/i
KIS_BUILD_DIR=${BUILDROOT}/kisbuild # only used for getting git sha number
KRITA_DMG=${BUILDROOT}/kritadmg
KRITA_DMG_TEMPLATE=${BUILDROOT}/kritadmg-template

export PATH=${KIS_INSTALL_DIR}/bin:$PATH

# flags for OSX environment
# We only support from 10.11 up
export MACOSX_DEPLOYMENT_TARGET=10.11
export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11


print_usage () {
    printf "USAGE: 
  osxdeploy.sh [-s=<identity>] [-notarize-ac=<apple-account>] [-style=<style.txt>] [-bg=<background-image>]

    -s \t\t\t Code sign identity for codesign

    -notarize-ac \t Apple account name for notarization purposes
\t\t\t script will attempt to get password from keychain, if fails provide one with
\t\t\t the -notarize-pass option: To add a password run 

\t\t\t   security add-generic-password -a \"AC_USERNAME\" -w <secret_password> -s \"KRITA_AC_PASS\"

    -notarize-pass \t If given, the Apple acount password. Otherwise an attempt will be macdeployqt_exists
\t\t\t to get the password from keychain using the account given in <notarize-ac> option.

    -style \t\t Style file defined from 'dmgstyle.sh' output

    -bg \t\t Set a background image for dmg folder.
\t\t\t osxdeploy needs an input image to attach to the dmg background
\t\t\t image recomended size is at least 950x500
"
}

# Attempt to detach previous mouted DMG
if [[ -d "/Volumes/${DMG_title}" ]]; then
    echo "WARNING: Another Krita DMG is mounted!"
    echo "Attempting eject…"
    hdiutil detach "/Volumes/${DMG_title}"
    if [ $? -ne 0  ]; then
        exit
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

    if [[ ${arg} = "-h" || ${arg} = "--help" ]]; then
        print_usage
        exit
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

    if [[ -n "${NOTARIZE_ACC}" ]]; then
        security find-generic-password -s "KRITA_AC_PASS" > /dev/null 2>&1
        if [[ ${?} -eq 0 || -n "${NOTARIZE_PASS}" ]]; then
            NOTARIZE="true"
        else
            echo "No password given for notarization or KRITA_AC_PASS missig in keychain"
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
    DMG_STYLE="${SCRIPT_SOURCE_DIR}/default.style"
fi

print_msg "Using style from: %s" "${DMG_STYLE}"

### Background for DMG
if [[ ${DMG_validBG} -eq 0 ]]; then
    echo "No jpg or png valid file detected!!"
    echo "Using default style"
    DMG_background="${SCRIPT_SOURCE_DIR}/krita_dmgBG.jpg"
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
    if test -z "$(grep ${1##*/} <<< ${llist})" ; then
        local llist="${llist} ${1##*/} "
    fi
    echo "${llist}"
}

# Find all @rpath and Absolute to buildroot path libs
# Add to libs_used
# converts absolute buildroot path to @rpath
find_needed_libs () {
    # echo "Analizing libraries with oTool..." >&2
    local libs_used="" # input lib_lists founded

    for libFile in ${@}; do
        if test -z "$(file ${libFile} | grep 'Mach-O')" ; then
            # echo "skipping ${libFile}" >&2
            continue
        fi

        oToolResult=$(otool -L ${libFile} | awk '{print $1}')
        resultArray=(${oToolResult}) # convert to array

        for lib in ${resultArray[@]:1}; do
            if test "${lib:0:1}" = "@"; then
                local libs_used=$(add_lib_to_list "${lib}" "${libs_used}")
            fi
            if [[ "${lib:0:${#BUILDROOT}}" = "${BUILDROOT}" ]]; then
                printf "Fixing %s: %s\n" "${libFile#${KRITA_DMG}/}" "${lib##*/}" >&2
                if [[ "${lib##*/}" = "${libFile##*/}" ]]; then
                    install_name_tool -id ${lib##*/} "${libFile}"
                else
                    install_name_tool -change ${lib} "@rpath/${lib##*${BUILDROOT}/i/lib/}" "${libFile}"
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
        if test -z "$(find ${KRITA_DMG}/krita.app/Contents/ -name ${lib})"; then
            # echo "Adding ${lib} to missing libraries." >&2
            libs_missing="${libs_missing} ${lib}"
        fi
    done
    echo "${libs_missing}"
}

copy_missing_libs () {
    for lib in ${@}; do
        result=$(find -L "${BUILDROOT}/i" -name "${lib}")

        if test $(countArgs ${result}) -eq 1; then
            if [ "$(stringContains "${result}" "plugin")" ]; then
                cp -pv ${result} ${KRITA_DMG}/krita.app/Contents/PlugIns/
                krita_findmissinglibs "${KRITA_DMG}/krita.app/Contents/PlugIns/${result##*/}"
            else
                cp -pv ${result} ${KRITA_DMG}/krita.app/Contents/Frameworks/
                krita_findmissinglibs "${KRITA_DMG}/krita.app/Contents/Frameworks/${result##*/}"
            fi
        else
            echo "${lib} might be a missing framework"
            if [ "$(stringContains "${result}" "framework")" ]; then
                echo "copying framework ${BUILDROOT}/i/lib/${lib}.framework to dmg"
                # rsync only included ${lib} Resources Versions
                rsync -priul ${BUILDROOT}/i/lib/${lib}.framework/${lib} ${KRITA_DMG}/krita.app/Contents/Frameworks/${lib}.framework/
                rsync -priul ${BUILDROOT}/i/lib/${lib}.framework/Resources ${KRITA_DMG}/krita.app/Contents/Frameworks/${lib}.framework/
                rsync -priul ${BUILDROOT}/i/lib/${lib}.framework/Versions ${KRITA_DMG}/krita.app/Contents/Frameworks/${lib}.framework/
                krita_findmissinglibs "$(find "${KRITA_DMG}/krita.app/Contents/Frameworks/${lib}.framework/" -type f -perm 755)"
            fi
        fi
    done
}

krita_findmissinglibs() {
    neededLibs=$(find_needed_libs "${@}")
    missingLibs=$(find_missing_libs ${neededLibs})

    if test $(countArgs ${missingLibs}) -gt 0; then
        printf "Found missing libs: %s\n" "${missingLibs}"
        copy_missing_libs ${missingLibs}
    fi
}

strip_python_dmginstall() {
    # reduce size of framework python
    # Removes tests, installers, pyenv, distutils
    echo "Removing unnecesary files from Python.Framework to be packaged..."
    PythonFrameworkBase="${KRITA_DMG}/krita.app/Contents/Frameworks/Python.framework"

    cd ${PythonFrameworkBase}
    find . -name "test*" -type d | xargs rm -rf
    find "${PythonFrameworkBase}/Versions/${PY_VERSION}/bin" -not -name "python*" \( -type f -or -type l \) | xargs rm -f
    cd "${PythonFrameworkBase}/Versions/${PY_VERSION}/lib/python${PY_VERSION}"
    rm -rf distutils tkinter ensurepip venv lib2to3 idlelib
}

# Some libraries require r_path to be removed
# we must not apply delete rpath globally
delete_install_rpath() {
    xargs -P4 -I FILE install_name_tool -delete_rpath "${BUILDROOT}/i/lib" FILE 2> "${BUILDROOT}/deploy_error.log"
}

fix_python_framework() {
    # Fix python.framework rpath and slims down installation
    PythonFrameworkBase="${KRITA_DMG}/krita.app/Contents/Frameworks/Python.framework"

    # Fix main library
    pythonLib="${PythonFrameworkBase}/Python"
    install_name_tool -id "${pythonLib##*/}" "${pythonLib}"
    install_name_tool -add_rpath @loader_path/../../../ "${pythonLib}" 2> /dev/null
    install_name_tool -change @loader_path/../../../../libintl.9.dylib @loader_path/../../../libintl.9.dylib "${pythonLib}"

    # Fix all executables
    install_name_tool -add_rpath @executable_path/../../../../../../../ "${PythonFrameworkBase}/Versions/Current/Resources/Python.app/Contents/MacOS/Python"
    install_name_tool -change "${KIS_INSTALL_DIR}/lib/Python.framework/Versions/${PY_VERSION}/Python" @executable_path/../../../../../../Python "${PythonFrameworkBase}/Versions/Current/Resources/Python.app/Contents/MacOS/Python"
    install_name_tool -add_rpath @executable_path/../../../../ "${PythonFrameworkBase}/Versions/Current/bin/python${PY_VERSION}"
    install_name_tool -add_rpath @executable_path/../../../../ "${PythonFrameworkBase}/Versions/Current/bin/python${PY_VERSION}m"

    # Fix rpaths from Python.Framework
    find ${PythonFrameworkBase} -type f -perm 755 | delete_install_rpath
    find "${PythonFrameworkBase}/Versions/Current/site-packages/PyQt5" -type f -name "*.so" | delete_install_rpath
}

# Checks for macdeployqt
# If not present attempts to install
# If it fails shows an informatve message
# (For now, macdeployqt is fundamental to deploy)
macdeployqt_exists() {
    printf "Checking for macdeployqt...  "

    if [[ ! -e "${KIS_INSTALL_DIR}/bin/macdeployqt" ]]; then
        printf "Not Found!\n"
        printf "Attempting to install macdeployqt\n"

        cd ${BUILDROOT}/depbuild/ext_qt/ext_qt-prefix/src/ext_qt/qttools/src
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
        exit
        else
            echo "Done!"
        fi
    else
        echo "Found!"
    fi
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

    mkdir -p ${KRITA_DMG}/krita.app/Contents/PlugIns
    mkdir -p ${KRITA_DMG}/krita.app/Contents/Frameworks

    echo "Copying share..."
    # Deletes old copies of translation and qml to be recreated
    cd ${KIS_INSTALL_DIR}/share/
    rsync -prul --delete ./ \
            --exclude krita_SRCS.icns \
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

    echo "Copying translations..."
    rsync -prul ${KIS_INSTALL_DIR}/translations/ \
            ${KRITA_DMG}/krita.app/Contents/Resources/translations

    echo "Copying kritaquicklook..."
    mkdir -p ${KRITA_DMG}/krita.app/Contents/Library/QuickLook
    rsync -prul ${KIS_INSTALL_DIR}/plugins/kritaquicklook.qlgenerator ${KRITA_DMG}/krita.app/Contents/Library/QuickLook

    cd ${KRITA_DMG}/krita.app/Contents
    ln -shF Resources share

    echo "Copying qml..."
    rsync -prul ${KIS_INSTALL_DIR}/qml Resources/qml

    echo "Copying plugins..."
    # exclude kritaquicklook.qlgenerator/
    cd ${KIS_INSTALL_DIR}/plugins/
    rsync -prul --delete --delete-excluded ./ \
        --exclude kritaquicklook.qlgenerator \
        ${KRITA_DMG}/krita.app/Contents/PlugIns

    cd ${BUILDROOT}
    rsync -prul ${KIS_INSTALL_DIR}/lib/kritaplugins/ ${KRITA_DMG}/krita.app/Contents/PlugIns
    
    # rsync -prul {KIS_INSTALL_DIR}/lib/libkrita* Frameworks/

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
        -qmldir=${KIS_INSTALL_DIR}/qml \
        # -extra-plugins=${KIS_INSTALL_DIR}/lib/kritaplugins \
        # -extra-plugins=${KIS_INSTALL_DIR}/lib/plugins \
        # -extra-plugins=${KIS_INSTALL_DIR}/plugins

    cd ${BUILDROOT}
    echo "macdeployqt done!"

    echo "Copying python..."
    # Copy this framework last!
    # It is best that macdeployqt does not modify Python.framework
    # folders with period in name are treated as Frameworks for codesign
    rsync -prul ${KIS_INSTALL_DIR}/lib/Python.framework ${KRITA_DMG}/krita.app/Contents/Frameworks/
    rsync -prul ${KIS_INSTALL_DIR}/lib/krita-python-libs ${KRITA_DMG}/krita.app/Contents/Frameworks/
    # change perms on Python to allow header change
    chmod +w ${KRITA_DMG}/krita.app/Contents/Frameworks/Python.framework/Python
    
    fix_python_framework
    strip_python_dmginstall

    # fix python pyc
    # precompile all pyc so the dont alter signature
    echo "Precompiling all python files..."
    cd ${KRITA_DMG}/krita.app
    ${KIS_INSTALL_DIR}/bin/python -m compileall . &> /dev/null

    install_name_tool -delete_rpath @loader_path/../../../../lib ${KRITA_DMG}/krita.app/Contents/MacOS/krita
    rm -rf ${KRITA_DMG}/krita.app/Contents/PlugIns/kf5/org.kde.kwindowsystem.platforms

    # repair krita for plugins
    printf "Searching for missing libraries\n"
    krita_findmissinglibs $(find ${KRITA_DMG}/krita.app/Contents -type f -perm 755 -or -name "*.dylib" -or -name "*.so")

    printf "removing absolute or broken linksys, if any\n"
    find "${KRITA_DMG}/krita.app/Contents" -type l \( -lname "/*" -or -not -exec test -e {} \; \) -print | xargs rm

    echo "Done!"

}


# helper to define function only once
batch_codesign() {
    xargs -P4 -I FILE codesign --options runtime --timestamp -f -s "${CODE_SIGNATURE}" FILE
}
# Code sign must be done as recommended by apple "sign code inside out in individual stages"
signBundle() {
    cd ${KRITA_DMG}

    # sign Frameworks and libs
    cd ${KRITA_DMG}/krita.app/Contents/Frameworks
    # remove debug version as both versions cant be signed.
    rm ${KRITA_DMG}/krita.app/Contents/Frameworks/QtScript.framework/Versions/Current/QtScript_debug
    find . -type f -perm 755 -or -name "*.dylib" -or -name "*.so" | batch_codesign
    find . -type d -name "*.framework" | xargs printf "%s/Versions/Current\n" | batch_codesign

    # Sign all other files in Framework (needed)
    # there are many files in python do we need to sign them all?
    find krita-python-libs -type f | batch_codesign
    # find python -type f | batch_codesign

    # Sing only libraries and plugins
    cd ${KRITA_DMG}/krita.app/Contents/PlugIns
    find . -type f | batch_codesign

    cd ${KRITA_DMG}/krita.app/Contents/Library/QuickLook
    printf "kritaquicklook.qlgenerator" | batch_codesign

    # It is recommended to sign every Resource file
    cd ${KRITA_DMG}/krita.app/Contents/Resources
    find . -type f | batch_codesign

    #Finally sign krita and krita.app
    printf "${KRITA_DMG}/krita.app/Contents/MacOS/krita" | batch_codesign
    printf "${KRITA_DMG}/krita.app" | batch_codesign
}

# Notarize build on macOS servers
# based on https://github.com/Beep6581/RawTherapee/blob/6fa533c40b34dec527f1176d47cc6c683422a73f/tools/osx/macosx_bundle.sh#L225-L250
notarize_build() {
    local NOT_SRC_DIR=${1}
    local NOT_SRC_FILE=${2}

    if [[ ${NOTARIZE} = "true" ]]; then
        printf "performing notarization of %s\n" "${2}"
        cd "${NOT_SRC_DIR}"
        
        if [[ -z "${NOTARIZE_PASS}" ]]; then
            NOTARIZE_PASS="@keychain:KRITA_AC_PASS"
        fi

        ditto -c -k --sequesterRsrc --keepParent "${NOT_SRC_FILE}" "${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip"

        # echo "xcrun altool --notarize-app --primary-bundle-id \"org.krita\" --username \"${NOTARIZE_ACC}\" --password \"${NOTARIZE_PASS}\" --file \"${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip\""
        local altoolResponse="$(xcrun altool --notarize-app --primary-bundle-id "org.krita" --username "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" --file "${BUILDROOT}/tmp_notarize/${NOT_SRC_FILE}.zip" 2>&1)"

        if [[ -n "$(grep 'Error' <<< ${altoolResponse})" ]]; then
            printf "ERROR: xcrun altool exited with the following error! \n\n%s\n\n" "${altoolResponse}"
            printf "This could mean there is an error in AppleID autentication!\n"
            printf "aborting notarization\n"
            NOTARIZE="false"
            return
        else
            printf "Response:\n\n%s\n\n" "${altoolResponse}"
        fi

        local uuid="$(grep 'RequestUUID' <<< ${altoolResponse} | awk '{ print $3 }')"
        echo "RequestUUID = ${uuid}" # Display identifier string

        waiting_fixed "Waiting to retrieve notarize status" 15

        while true ; do
            fullstatus=$(xcrun altool --notarization-info "${uuid}" --username "${NOTARIZE_ACC}" --password "${NOTARIZE_PASS}" 2>&1)  # get the status
            notarize_status=`echo "${fullstatus}" | grep 'Status\:' | awk '{ print $2 }'`
            echo "${fullstatus}"
            if [[ "${notarize_status}" = "success" ]]; then
                xcrun stapler staple "${NOT_SRC_FILE}"   #staple the ticket
                xcrun stapler validate -v "${NOT_SRC_FILE}"
                print_msg "Notarization success!"
                break
            elif [[ "${notarize_status}" = "in" ]]; then
                waiting_fixed "Notarization still in progress, sleeping for 15 seconds and trying again" 15
            else
                echo "Notarization failed! full status below"
                echo "${fullstatus}"
                exit 1
            fi
        done
    fi
}

createDMG () {
    printf "Creating of dmg with contents of %s...\n" "${KRITA_DMG}"
    cd ${BUILDROOT}
    DMG_size=700

    ## Build dmg from folder

    # create dmg on local system
    # usage of -fsargs minimze gaps at front of filesystem (reduce size)
    hdiutil create -srcfolder "${KRITA_DMG}" -volname "${DMG_title}" -fs HFS+ \
        -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${DMG_size}m krita.temp.dmg

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

    ## Apple script to set style
    style="$(<"${DMG_STYLE}")"
    printf "${style}" "${DMG_title}" "${DMG_background##*/}" | osascript

    #Set Icon for DMG
    cp -v "${SCRIPT_SOURCE_DIR}/KritaIcon.icns" "/Volumes/${DMG_title}/.VolumeIcon.icns"
    SetFile -a C "/Volumes/${DMG_title}"
    
    chmod -Rf go-w "/Volumes/${DMG_title}"

    # ensure all writting operations to dmg are over
    sync

    hdiutil detach $device
    hdiutil convert "krita.temp.dmg" -format UDZO -imagekey -zlib-level=9 -o krita-out.dmg

    # Add git version number
    GIT_SHA=$(grep "#define KRITA_GIT_SHA1_STRING" ${KIS_BUILD_DIR}/libs/version/kritagitversion.h | awk '{gsub(/"/, "", $3); printf $3}')

    mv krita-out.dmg krita-nightly_${GIT_SHA}.dmg
    echo "moved krita-out.dmg to krita-nightly_${GIT_SHA}.dmg"
    rm krita.temp.dmg

    if [[ -n "${CODE_SIGNATURE}" ]]; then
        printf "krita-nightly_${GIT_SHA}.dmg" | batch_codesign
    fi

    notarize_build ${BUILDROOT} "krita-nightly_${GIT_SHA}.dmg"

    echo "dmg done!"
}

#######################
# Program starts!!
########################
# Run deploy command, instalation is assumed to exist in BUILDROOT/i
krita_deploy

# Code sign krita.app if signature given
if [[ -n "${CODE_SIGNATURE}" ]]; then
    signBundle
fi

notarize_build ${KRITA_DMG} krita.app

# Create DMG from files inside ${KRITA_DMG} folder
createDMG

if [[ "${NOTARIZE}" = "false" ]]; then
    macosVersion="$(sw_vers | grep ProductVersion | awk '
       BEGIN { FS = "[ .\t]" }
             { print $3}
    ')"
    if (( ${macosVersion} == 15 )); then
        print_error "Build not notarized! Needed for macOS versions above 10.14"
    fi
fi

# signal end of script
if [[ -n "$TERM" ]]; then
    tput bel
fi
