SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 

if(MESON_BINARY_PATH AND NINJA_BINARY_PATH)

    ExternalProject_Add(
        ext_raqm
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/therahedwig/libraqm/archive/793d7a5e89f3a2bdda8d9236d24294e48dc2e354.zip
        URL_HASH SHA256=576e63009a26d613aab8dd3bd216be43a49979d26aa99cb186a0008a9f3af252
        
        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/fix_utf_16.patch

        CONFIGURE_COMMAND meson <SOURCE_DIR> --prefix=${EXTPREFIX_raqm} --libdir=lib

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
