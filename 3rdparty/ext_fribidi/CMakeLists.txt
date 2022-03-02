SET(EXTPREFIX_fribidi "${EXTPREFIX}" ) 

find_program(MESON_BINARY_PATH meson)
find_program(NINJA_BINARY_PATH ninja)

if(MESON_BINARY_PATH AND NINJA_BINARY_PATH)

    ExternalProject_Add(
        ext_raqm
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/fribidi/fribidi/releases/download/v1.0.11/fribidi-1.0.11.tar.xz
        URL_HASH SHA256=30f93e9c63ee627d1a2cedcf59ac34d45bf30240982f99e44c6e015466b4e73d

        CONFIGURE_COMMAND meson <SOURCE_DIR> --prefix=${EXTPREFIX_fribidi}

        BUILD_COMMAND meson compile -j${SUBMAKE_JOBS}

        INSTALL_COMMAND meson install
    )
else()
    if (NOT MESON_BINARY_PATH)
        message(WARNING "Meson Build system was not found, see https://mesonbuild.com")
    endif()
    if (NOT NINJA_BINARY_PATH)
        message(WARNING "Ninja Build system was not found, see https://ninja-build.org/")
    endif()
endif()
