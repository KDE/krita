#!/usr/bin/env sh

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
    

DMG_title="krita" #if changed krita.temp.dmg must be deleted manually

if test -z ${BUILDROOT}; then
    echo "ERROR: BUILDROOT env not set!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    echo "exiting..."
    exit
fi

# There is some duplication between build and deploy scripts
# a config env file could would be a nice idea.
KIS_INSTALL_DIR=${BUILDROOT}/i
KIS_BUILD_DIR=${BUILDROOT}/kisbuild # only used for getting git sha number
KRITA_DMG=${BUILDROOT}/kritadmg
KRITA_DMG_TEMPLATE=${BUILDROOT}/kritadmg-template

# flags for OSX environment
# We only support from 10.11 up
export MACOSX_DEPLOYMENT_TARGET=10.11
export QMAKE_MACOSX_DEPLOYMENT_TARGET=10.11

print_usage () {
    echo "USAGE: osxdeploy.sh <background-image> [pkg]"
    echo "\t osxdeploy needs an input image to add to the dmg background
    \t image recomended size is at least 950x500\n"
}

if test ${#} -eq 0; then
    echo "ERROR: no option given"
    print_usage
    exit 1
fi

if [[ -f ${1} ]]; then
    DMG_validBG=0
    echo "attempting to check background is valid jpg or png..."
    BG_FORMAT=$(sips --getProperty format ${1} | awk '{printf $2}')
    
    if [[ "png" = ${BG_FORMAT} || "jpeg" = ${BG_FORMAT} ]];then
        echo "valid image file"
        DMG_background=${1}
        DMG_validBG=1
    fi
fi

if [[ ${DMG_validBG} -eq 0 ]]; then
    echo "No jpg or png valid file detected!!"
    echo "exiting"
    exit
fi

countArgs () {
    echo "${#}"
}

stringContains () {
    echo "$(grep "${2}" <<< "${1}")"

}

find_needed_libs () {
    local libs_used=""
    for soFile in $(find ${KRITA_DMG}/krita.app/Contents/PlugIns -name "*.so" -or -name "*.dylib"); do
        soFy=$(otool -L ${soFile} | cut -d " " -f 1)
        echo "collecting libs from ${soFile}…" >&2
        for lib in ${soFy}; do
            if test "${lib:0:1}" = "@"; then
                if test -z "$(grep ${lib##*/} <<< ${libs_used})" ; then
                    local libs_used="${libs_used} ${lib##*/} "
                fi
            fi
        done
    done
    echo ${libs_used}
}

find_missing_libs (){
    echo "Searching for missing libs on deployment folders…" >&2
    local libs_missing=""
    for lib in ${@}; do
        if test -z "$(find ${BUILDROOT}/kritadmg/krita.app/Contents/ -name ${lib})"; then
            echo "Adding ${lib} to missing libraries." >&2
            libs_missing="${libs_missing} ${lib}"
        fi
    done
    echo ${libs_missing}
}

copy_missing_libs () {
    for lib in ${@}; do
        result=$(find "${BUILDROOT}/i" -name "${lib}")

        if test $(countArgs ${result}) -eq 1; then
            echo ${result}
            if [ "$(stringContains "${result}" "plugin")" ]; then
                echo "copying ${lib} to plugins dir"
                cp -pv ${result} ${KRITA_DMG}/krita.app/Contents/PlugIns/
            else
                echo "copying ${lib} to Frameworks dir"
                cp -pv ${result} ${KRITA_DMG}/krita.app/Contents/Frameworks/
            fi
        else
            echo "${lib} might be a missing framework"
            if [ "$(stringContains "${result}" "framework")" ]; then
                echo "copying framework ${BUILDROOT}/i/lib/${lib}.framework to dmg"
                cp -pvr ${BUILDROOT}/i/lib/${lib}.framework ${KRITA_DMG}/krita.app/Contents/Frameworks/
            fi
        fi
    done
}

krita_findmissinglibs() {
    echo "Adding missin glibraries for plugins"
    neededLibs=$(find_needed_libs)
    missingLibs=$(find_missing_libs ${neededLibs})

    if test $(countArgs ${missingLibs}) -gt 0; then
        echo "Found missing libs!"
        echo "${missingLibs}\n"
        copy_missing_libs ${missingLibs}
    else
        echo "No missing libraries found."
    fi

    echo "Done!"
}


krita_deploy () {
    # fix_boost_rpath

    cd ${BUILDROOT}
    # Update files in krita.app
    rm -rf ./krita.dmg ${KRITA_DMG}
    # Copy new builtFiles
    echo "Copying krita.app..."
    rsync -riul ${KRITA_DMG_TEMPLATE}/ ${KRITA_DMG}
    rsync -priul ${KIS_INSTALL_DIR}/bin/krita.app ${KRITA_DMG}

    echo "Copying share..."
    # Deletes old copies of translation and qml to be recreated
    cd ${KIS_INSTALL_DIR}/share/
    rsync -priul --delete --delete-excluded ./ \
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
    rsync -priul ${KIS_INSTALL_DIR}/translations/ \
            ${KRITA_DMG}/krita.app/Contents/Resources/translations

    echo "Copying kritaquicklook..."
    mkdir -p ${KRITA_DMG}/krita.app/Contents/Library/QuickLook
    rsync -priul ${KIS_INSTALL_DIR}/plugins/kritaquicklook.qlgenerator ${KRITA_DMG}/krita.app/Contents/Library/QuickLook

    cd ${KRITA_DMG}/krita.app/Contents
    ln -shF Resources share

    echo "Copying qml..."
    rsync -priul ${KIS_INSTALL_DIR}/qml Resources/qml

    echo "Copying plugins..."
    mkdir -p ${KRITA_DMG}/krita.app/Contents/PlugIns
    # exclude kritaquicklook.qlgenerator/
    cd ${KIS_INSTALL_DIR}/plugins/
    rsync -priul --delete --delete-excluded ./ \
        --exclude kritaquicklook.qlgenerator \
        ${KRITA_DMG}/krita.app/Contents/PlugIns

    cd ${BUILDROOT}
    rsync -priul ${KIS_INSTALL_DIR}/lib/kritaplugins/ ${KRITA_DMG}/krita.app/Contents/PlugIns
    
    # rsync -prul /Volumes/Osiris/programs/krita-master/i/lib/libkrita* Frameworks/

    # activate for python enabled Krita
    # echo "Copying python..."
    # cp -r ${KIS_INSTALL_DIR}/lib/python3.5 Frameworks/
    # cp -r ${KIS_INSTALL_DIR}/lib/krita-python-libs Frameworks/

    # XXX: fix rpath for krita.so
    # echo "Copying sip..."
    # rsync -Prvul ${KIS_INSTALL_DIR}/sip Frameworks/

    # To avoid errors macdeployqt must be run from bin location
    # ext_qt will not build macdeployqt by default so it must be build manually
    #   cd ${BUILDROOT}/depbuild/ext_qt/ext_qt-prefix/src/ext_qt/qttools/src
    #   make sub-macdeployqt-all
    #   make sub-macdeployqt-install_subtargets
    #   make install
    echo "Preparing ${KRITA_DMG} for deployment..."
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
    install_name_tool -delete_rpath @loader_path/../../../../lib ${KRITA_DMG}/krita.app/Contents/MacOS/krita
    rm -rf ${KRITA_DMG}/krita.app/Contents/PlugIns/kf5/org.kde.kwindowsystem.platforms

    # repair krita for plugins
    krita_findmissinglibs
}

createDMG () {
    echo "Starting creation of dmg..."
    cd ${BUILDROOT}
    DMG_size=500

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
    cp ${BUILDROOT}/${DMG_background} "/Volumes/${DMG_title}/.background/"

    ## Apple script to set style
    echo '
        tell application "Finder"
            tell disk "'${DMG_title}'"
                open
                set current view of container window to icon view
                set toolbar visible of container window to false
                set statusbar visible of container window to false
                set the bounds of container window to {186, 156, 956, 592}
                set theViewOptions to the icon view options of container window
                set arrangement of theViewOptions to not arranged
                set icon size of theViewOptions to 80
                set background picture of theViewOptions to file ".background:'${DMG_background}'"
                set position of item "'krita.app'" of container window to {279, 272}
                set position of item "Applications" of container window to {597, 272}
                set position of item "Terms of Use" of container window to {597, 110}
                update without registering applications
                delay 1
                close
            end tell
        end tell
        ' | osascript

    
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

# Run deploy command, instalation is assumed to exist in BUILDROOT/i
# krita_deploy

# Create DMG from files insiede ${KRITA_DMG} folder
createDMG
