SET(EXTPREFIX_harfbuzz "${EXTPREFIX}" ) 

if(MESON_BINARY_PATH AND NINJA_BINARY_PATH)

    ExternalProject_Add(
        ext_harfbuzz
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/harfbuzz/harfbuzz/releases/download/4.1.0/harfbuzz-4.1.0.tar.xz
        URL_HASH SHA256=f7984ff4241d4d135f318a93aa902d910a170a8265b7eaf93b5d9a504eed40c8

        CONFIGURE_COMMAND meson <SOURCE_DIR> --prefix=${EXTPREFIX_harfbuzz}

        BUILD_COMMAND meson compile -j${SUBMAKE_JOBS}

        INSTALL_COMMAND meson install
        
        DEPENDS ext_freetype
    )
else()
    if (NOT MESON_BINARY_PATH)
        message(WARNING "Meson Build system was not found, see https://mesonbuild.com")
    endif()
    if (NOT NINJA_BINARY_PATH)
        message(WARNING "Ninja Build system was not found, see https://ninja-build.org/")
    endif()
endif()
