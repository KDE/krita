#!/usr/bin/env bash

# Krita tool to create dmg from installed source
# Copies all files to a folder to be converted into the final dmg
# Background image must be set for it to deploy correcly

# osxdeploy.sh automates the creation of the release DMG. It needs an image
#     either png or jpg as first argument as it will use the image to set
#     the background of the DMG.

# Necessary files can be downloaded from https://drive.google.com/drive/folders/15cUhCou7ya9ktjfhbzRaL7_IpntzxG4j?usp=sharing

# A short explanation of what it does:

# - Creates a copy of "krita-template" folder (this containes Terms of use
#     and Applications) into kritadmg folder.
# Application folder symlink can be created with applescript but Terms of use contents cannot,
# also working like this allows to add other files to dmg if needed.
# Place the folder in ${BUILDROOT}

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
KIS_INSTALL_DIR=${BUILDROOT}/i
KIS_BUILD_DIR=${BUILDROOT}/kisbuild # only used for getting git sha number
KRITA_DMG=${BUILDROOT}/kritadmg
KRITA_DMG_TEMPLATE=${BUILDROOT}/kritadmg-template

export PATH=${KIS_INSTALL_DIR}/bin:$PATH

# flags for OSX environment
# We only support from 10.11 up
export MACOSX_DEPLOYMENT_TARGET=10.11
export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11

# Attempt to find python_version
local_PY_MAYOR_VERSION=$(python -c "import sys; print(sys.version_info[0])")
local_PY_MINOR_VERSION=$(python -c "import sys; print(sys.version_info[1])")
PY_VERSION="${local_PY_MAYOR_VERSION}.${local_PY_MINOR_VERSION}"
echo "Detected Python ${PY_VERSION}"

print_usage () {
    echo "USAGE: osxdeploy.sh [-s=<identity>] [-style=<style.txt>] <background-image>"
    echo "\t -s Code sign identity for codesign"
    echo "\t -style Style file defined from 'dmgstyle.sh' output"
    echo "\t osxdeploy needs an input image to add to the dmg background
    \t image recomended size is at least 950x500\n"
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

# Parse input args
if test ${#} -eq 0; then
    echo "ERROR: no option given"
    print_usage
    exit 1
fi

for arg in "${@}"; do
    if [[ -f ${arg} ]]; then
        DMG_validBG=0
        echo "attempting to check background is valid jpg or png..."
        BG_FORMAT=$(sips --getProperty format ${arg} | awk '{printf $2}')

        if [[ "png" = ${BG_FORMAT} || "jpeg" = ${BG_FORMAT} ]];then
            echo "valid image file"
            DMG_background=$(cd "$(dirname "${arg}")"; pwd -P)/$(basename "${arg}")
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

if [[ ! ${DMG_STYLE} ]]; then
    DMG_STYLE="${SCRIPT_SOURCE_DIR}/default.style"
fi
echo "Using style from: ${DMG_STYLE}"

if [[ ${DMG_validBG} -eq 0 ]]; then
    echo "No jpg or png valid file detected!!"
    echo "exiting"
    exit
fi


if [[ -z "${CODE_SIGNATURE}" ]]; then
    echo "WARNING: No signature provided, Code will not be signed"
else
    printf 'Code will be signed with "%s"\n' "${CODE_SIGNATURE}"
fi

# Helper functions
countArgs () {
    echo "${#}"
}

stringContains () {
    echo "$(grep "${2}" <<< "${1}")"
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
                krita_findmissinglibs "$(find "${KRITA_DMG}/krita.app/Contents/Frameworks/${lib}.framework/" -perm u+x)"
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
    find "${PythonFrameworkBase}/Versions/${PY_VERSION}/bin" -not -name "python*" | xargs rm -f
    cd "${PythonFrameworkBase}/Versions/${PY_VERSION}/lib/python${PY_VERSION}"
    rm -rf distutils tkinter ensurepip venv lib2to3 idlelib
}

fix_python_framework() {
    # Fix python.framework rpath and slims down installation
    # fix library LD_RPATH excutable_path and loader_path.
    # It is intended to be used for Libraries inside Frameworks.
    fix_framework_library() {
        xargs -P4 -I FILE sh -c "
            install_name_tool -rpath ${KIS_INSTALL_DIR}/lib @loader_path/Frameworks \"${libFile}\" 2> /dev/null
            install_name_tool -add_rpath @loader_path/../../../ \"${libFile}\" 2> /dev/null
        "
    }
    # Start fixing all executables
    PythonFrameworkBase="${KRITA_DMG}/krita.app/Contents/Frameworks/Python.framework"
    install_name_tool -change @loader_path/../../../../libintl.9.dylib @loader_path/../../../libintl.9.dylib "${PythonFrameworkBase}/Python"
    install_name_tool -add_rpath @executable_path/../../../../../../../ "${PythonFrameworkBase}/Versions/Current/Resources/Python.app/Contents/MacOS/Python"
    install_name_tool -add_rpath @executable_path/../../../../ "${PythonFrameworkBase}/Versions/Current/bin/python${PY_VERSION}"
    install_name_tool -add_rpath @executable_path/../../../../ "${PythonFrameworkBase}/Versions/Current/bin/python${PY_VERSION}m"

    # Fix rpaths from Python.Framework
    # install_name_tool change @loader_path/../../../libz.1.dylib

    # Fix main library
    printf ${PythonFrameworkBase}/Python | fix_framework_library
    # find ${PythonFrameworkBase} -name "*.so" -not -type l | fix_framework_library
}

krita_deploy () {
    # fix_boost_rpath

    cd ${BUILDROOT}
    # Update files in krita.app
    echo "Deleting previous kritadmg run..."
    rm -rf ./krita.dmg ${KRITA_DMG}
    # Copy new builtFiles
    echo "Preparing ${KRITA_DMG} for deployment..."

    echo "Copying krita.app..."
    rsync -riul ${KRITA_DMG_TEMPLATE}/ ${KRITA_DMG}
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
    krita_findmissinglibs $(find ${KRITA_DMG}/krita.app/Contents -type f -name "*.dylib" -or -name "*.so" -or -perm u+x)
    echo "Done!"

}

# helper to define function only once
batch_codesign() {
    xargs -P4 -I FILE codesign -f -s "${CODE_SIGNATURE}" FILE
}
# Code sign must be done as recommended by apple "sign code inside out in individual stages"
signBundle() {
    cd ${KRITA_DMG}

    # sign Frameworks and libs
    cd ${KRITA_DMG}/krita.app/Contents/Frameworks
    # remove debug version as both versions cant be signed.
    rm ${KRITA_DMG}/krita.app/Contents/Frameworks/QtScript.framework/Versions/Current/QtScript_debug
    find . -type f -name "*.dylib" -or -name "*.so" | batch_codesign
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
# Create DMG from files insiede ${KRITA_DMG} folder
createDMG
