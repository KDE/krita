#!/usr/bin/env sh

# get krita disk mounted image current properties
# creates a dummy folder to aid in setting the layout for
# the dmg image file.

set_dmgfolder_props() {
    cp ${DMG_background} "${DMG_STYLE_DIR}/.background"
    BG_FILENAME=${DMG_background##*/}
    printf '
    tell application "Finder"
        tell folder "%s" of %s
            open
            set current view of container window to icon view
            set toolbar visible of container window to false
            set statusbar visible of container window to false
            set the bounds of container window to {186, 156, 956, 592}
            set theViewOptions to the icon view options of container window
            set arrangement of theViewOptions to not arranged
            set icon size of theViewOptions to 80
            set background picture of theViewOptions to file ".background:%s"
            set position of item "krita.app" of container window to {279, 272}
            set position of item "Applications" of container window to {597, 272}
            set position of item "Terms of Use" of container window to {597, 110}
            update without registering applications
            close
        end tell
    end tell
    ' "${DMG_STYLE_DIR_OSA}" "${DMG_STYLE_LOCATION}" "${BG_FILENAME}" | osascript

    get_dmgfolder_props > /dev/null
}

get_dmgfolder_props() {
    printf '
        tell application "Finder"
            tell folder "%s" of %s
                open
                set v_cv to current view of container window
                log v_cv
                set v_tb to toolbar visible of container window
                log v_tb
                set v_stat to statusbar visible of container window
                log v_stat
                set v_bounds to the bounds of container window
                log v_bounds
                set theViewOptions to the icon view options of container window
                set v_arr to arrangement of theViewOptions
                log v_arr
                set v_icons to icon size of theViewOptions
                log v_icons
                set v_pos_k to position of item "krita.app" of container window
                log v_pos_k
                set v_pos_ap to position of item "Applications" of container window
                log v_pos_ap
                set v_pos_term to position of item "Terms of Use" of container window
                log v_pos_term
                update without registering applications
            end tell
        end tell
        ' "${DMG_STYLE_DIR_OSA}" "${DMG_STYLE_LOCATION}" | osascript 2>&1 | awk '{printf "%s\n" ,$0}'
}

# Print Suitable command to paste on osxdeploy.sh
print_osascript_cmd() {
    OLD_IFS=${IFS}
    IFS=$'\n'
    dmg_ops=($@)
    printf 'tell application "Finder"
    tell disk "%%s"
        open
        set current view of container window to %s
        set toolbar visible of container window to %s
        set statusbar visible of container window to %s
        set the bounds of container window to {%s}
        set theViewOptions to the icon view options of container window
        set arrangement of theViewOptions to %s
        set icon size of theViewOptions to %s
        set background picture of theViewOptions to file ".background:%%s"
        set position of item "krita.app" of container window to {%s}
        set position of item "Applications" of container window to {%s}
        set position of item "Terms of Use" of container window to {%s}
        update without registering applications
        delay 1
        close
    end tell
end tell
' "${dmg_ops[@]}"
    IFS=${OLD_IFS}
}

print_help() {
    printf \
"dmgstyle.sh is a minitool to aid in the creation of the DMG look for krita
out is designed to be copy pasted into osxdeploy.sh in the osasrcipt section

- Run once with set to set new background
- Edit by moving elements or <Cmd> + J to set net icon size
- To get current options for osxdeploy: rerun with no arguments.

USAGE: dmgstyle.sh [option] <args>
Options:
<empty>     No option fetches dummy design window state for paste in osxdeploy
set         set default values changing the background to image in <args>
"
}

if test -z ${BUILDROOT}; then
    echo "ERROR: BUILDROOT env not set!"
    echo "\t Must point to the root of the buildfiles as stated in 3rdparty Readme"
    print_help
    exit
fi

BUILDROOT=$(cd "$(dirname "${BUILDROOT}")"; pwd -P)/$(basename "${BUILDROOT}") # force full path

# if in home we get path minus $HOME
# if in Mounted disk we get path minus "/Volumes/diskname"
if [[ -n "$(grep /Users/$(whoami) <<< ${BUILDROOT})" ]];then
    # buildroot is in home
    build_folder=${BUILDROOT#${HOME}/} # removes home
    dmg_vol_name="home"
else
    tmp_folder=${BUILDROOT#/Volumes/}
    build_folder=${tmp_folder#*/} # get minus all
    printf -v dmg_vol_name 'disk "%s"' ${tmp_folder%%/*} # get volume name
fi


# Setting global variables
DMG_DUMMY_NAME="dmg_style_dummy"
DMG_STYLE_DIR="${BUILDROOT}/${DMG_DUMMY_NAME}"
DMG_STYLE_DIR_OSA=$(printf "%s/%s" ${build_folder} ${DMG_DUMMY_NAME} | tr / :)
DMG_STYLE_LOCATION=${dmg_vol_name}
KIS_INSTALL_DIR="${BUILDROOT}/i"


if [[ ${1} = "-h" || ${1} = "--help" ]]; then
    print_help
    exit
fi

if [[ ${1} = "set" ]]; then
    # Create style container folder it if doesn't exist
    if [[ ! -e ${DMG_STYLE_DIR} ]]; then
        mkdir "${DMG_STYLE_DIR}"
        mkdir "${DMG_STYLE_DIR}/Terms of Use"
        mkdir "${DMG_STYLE_DIR}/.background"
        ln -s "/Applications" "${DMG_STYLE_DIR}/Applications"
        ln -s "${KIS_INSTALL_DIR}/bin/krita.app" "${DMG_STYLE_DIR}/krita.app"
    fi

    ## check background validity
    if [[ -f ${2} ]]; then
        DMG_validBG=0
        echo "attempting to check background is valid jpg or png..."
        BG_FORMAT=$(sips --getProperty format ${2} | awk '{printf $2}')

        if [[ "png" = ${BG_FORMAT} || "jpeg" = ${BG_FORMAT} ]];then
            echo "valid image file"
            DMG_background=$(cd "$(dirname "${2}")"; pwd -P)/$(basename "${2}")
            DMG_validBG=1
        fi
    fi

    if [[ ${DMG_validBG} -eq 0 ]]; then
        echo "No jpg or png given or valid file detected!!"
        exit
    else
        BG_DPI=$(sips --getProperty dpiWidth ${DMG_background} | grep dpi | awk '{print $2}')
        if [[ $(echo "${BG_DPI} > 150" | bc -l) -eq 1 ]]; then
        printf "WARNING: image dpi has an effect on apparent size!
Check dpi is adequate for screen display if image appears very small
Current dpi is: %s\n" ${BG_DPI}
        fi
        set_dmgfolder_props
    fi

else
    if [[ ! -e ${DMG_STYLE_DIR} ]]; then
        echo "First run must use option [set]!!"
        exit
    fi
    osa_values=$(get_dmgfolder_props)
    print_osascript_cmd "${osa_values}"
fi
