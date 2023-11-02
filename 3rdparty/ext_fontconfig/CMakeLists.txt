SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )


kis_ExternalProject_Add_with_separate_builds_apple(
    ext_fontconfig
    MESON
    URL https://freedesktop.org/software/fontconfig/release/fontconfig-2.14.2.tar.gz
    URL_HASH SHA256=3ba2dd92158718acec5caaf1a716043b5aa055c27b081d914af3ccb40dce8a55

    #PATCH_COMMAND 
        #${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-fontconfig-Enable-tool-CLI-option-handling-24553.patch
        #COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-fontconfig-Do-not-create-symlinks-microsoft-23735.patch
        #COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-fontconfig-Enable-tool-CLI-option-handling-24553.patch
        #COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Add-default-fonts-path-for-Android.patch
        #COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Always-find-files-when-using-DESTDIR.patch

    CONFIGURE_ARGS --prefix=${PREFIX_ext_fontconfig}
        -Ddoc=disabled
        -Dcache-build=disabled
        -Dtests=disabled
        ${EXTRA_MESON_FLAGS}

    UPDATE_COMMAND ""

    DEPENDS ext_freetype ${GETTEXT_DEP} ${MESON_DEP} ${PKG_CONFIG_DEP}
)
