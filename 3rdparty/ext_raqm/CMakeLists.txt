SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 

find_program(MESON_BINARY_PATH meson)
find_program(NINJA_BINARY_PATH ninja)

if(MESON_BINARY_PATH AND NINJA_BINARY_PATH)

    ExternalProject_Add(
        ext_raqm
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/HOST-Oman/libraqm/archive/refs/tags/v0.9.0.tar.gz
        URL_HASH SHA256=826e43bc638b53ec720e93a26f4ead494c6a28006d280609dac6aef09b39283e

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
