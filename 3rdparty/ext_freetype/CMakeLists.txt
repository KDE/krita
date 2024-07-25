SET(PREFIX_ext_freetype "${EXTPREFIX}" )
SET(EXTPREFIX_harfbuzz "${EXTPREFIX}" )

if (POLICY CMP0114)
    cmake_policy(SET CMP0114 NEW)
endif()

set(FREETYPE_URL https://downloads.sourceforge.net/project/freetype/freetype2/2.12.1/freetype-2.12.1.tar.xz)
set(FREETYPE_SHA256 4766f20157cc4cf0cd292f80bf917f92d1c439b243ac3018debf6b9140c41a7f)

set(FREETYPE_SOURCE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/freetype_src")
file(MAKE_DIRECTORY ${FREETYPE_SOURCE_DIRECTORY})
file(WRITE ${FREETYPE_SOURCE_DIRECTORY}/foobar "yet-to-be-deleted-and-repopulated-at-build-stage")

ExternalProject_Add(
    ext_freetype_src
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    URL ${FREETYPE_URL}
    URL_HASH SHA256=${FREETYPE_SHA256}

    SOURCE_DIR ${FREETYPE_SOURCE_DIRECTORY}

    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

ExternalProject_Add(ext_freetype_bootstrap
    SOURCE_DIR ${FREETYPE_SOURCE_DIRECTORY}

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

    DEPENDS ext_png ext_freetype_src
)

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_harfbuzz
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/harfbuzz/harfbuzz/releases/download/5.3.1/harfbuzz-5.3.1.tar.xz
    URL_HASH SHA256=4a6ce097b75a8121facc4ba83b5b083bfec657f45b003cd5a3424f2ae6b4434d

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/harfbuzz-clang16-fix.patch

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
    SOURCE_DIR ${FREETYPE_SOURCE_DIRECTORY}

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
    DEPENDS ext_harfbuzz ext_png ext_freetype_src
)
