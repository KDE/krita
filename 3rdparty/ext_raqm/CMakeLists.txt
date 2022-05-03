SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 

if(MESON_BINARY_PATH AND NINJA_BINARY_PATH)

    ExternalProject_Add(
        ext_raqm
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/HOST-Oman/libraqm/archive/refs/heads/master.zip
        URL_HASH SHA256=fb9d0a9c9269299a85b6bcb39cd49915305dbc86e182431f1e2544856a6bcace
        
        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Implement-letter-and-word-spacing.patch
            COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-add_font_feature-range.patch

        CONFIGURE_COMMAND meson <SOURCE_DIR> --prefix=${EXTPREFIX_raqm}

        BUILD_COMMAND meson compile -j${SUBMAKE_JOBS}

        INSTALL_COMMAND meson install

        DEPENDS ext_freetype
        DEPENDS ext_harfbuzz
        DEPENDS ext_fribidi
    )
else()
    if (NOT MESON_BINARY_PATH)
        message(WARNING "Meson Build system was not found, see https://mesonbuild.com")
    endif()
    if (NOT NINJA_BINARY_PATH)
        message(WARNING "Ninja Build system was not found, see https://ninja-build.org/")
    endif()
endif()
