SET(PREFIX_ext_freetype "${EXTPREFIX}" )
SET(EXTPREFIX_harfbuzz "${EXTPREFIX}" )

if (POLICY CMP0114)
    cmake_policy(SET CMP0114 NEW)
endif()

set(FREETYPE_URL https://downloads.sourceforge.net/project/freetype/freetype2/2.13.2/freetype-2.13.2.tar.xz)
set(FREETYPE_SHA256 12991c4e55c506dd7f9b765933e62fd2be2e06d421505d7950a132e4f1bb484d)

ExternalProject_Add(ext_freetype_bootstrap
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    URL ${FREETYPE_URL}
    URL_HASH SHA256=${FREETYPE_SHA256}

    INSTALL_DIR ${PREFIX_ext_freetype}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_freetype}
        -DFT_DISABLE_BZIP2=ON
        -DFT_DISABLE_BROTLI=ON
        -DFT_REQUIRE_PNG=ON
        -DFT_DISABLE_HARFBUZZ=ON
        -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=TRUE
        ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""

    DEPENDS ext_png
)

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_harfbuzz
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/harfbuzz/harfbuzz/releases/download/8.1.0/harfbuzz-8.1.0.tar.xz
    URL_HASH SHA256=639596da7c26cc503b16b3bf4dfdca77fe832d3c9e06004ef63a7ce53cafbac9

    CONFIGURE_ARGS --prefix=${EXTPREFIX_harfbuzz}
        -Dglib=disabled
        -Dgobject=disabled
        -Dtests=disabled
        -Dfreetype=enabled
        ${EXTRA_MESON_FLAGS}

    UPDATE_COMMAND ""

    DEPENDS ext_freetype_bootstrap
)

# Amyspark: just run roughshod!
ExternalProject_Add( ext_freetype
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL ${FREETYPE_URL}
    URL_HASH SHA256=${FREETYPE_SHA256}

    INSTALL_DIR ${PREFIX_ext_freetype}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DFT_DISABLE_BZIP2=ON
        -DFT_DISABLE_BROTLI=ON
        -DFT_REQUIRE_HARFBUZZ=ON
        -DFT_REQUIRE_PNG=ON
        -DBUILD_SHARED_LIBS=TRUE
        ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ext_harfbuzz ext_png
)
