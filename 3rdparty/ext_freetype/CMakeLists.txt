SET(PREFIX_ext_freetype "${EXTPREFIX}" )
SET(EXTPREFIX_harfbuzz "${EXTPREFIX}" )

if (POLICY CMP0114)
    cmake_policy(SET CMP0114 NEW)
endif()

set(FREETYPE_URL https://download.savannah.gnu.org/releases/freetype/freetype-2.11.1.tar.xz)
set(FREETYPE_SHA256 3333ae7cfda88429c97a7ae63b7d01ab398076c3b67182e960e5684050f2c5c8)

set (BASE_FREETYPE_TARGET ext_freetype)

if(MESON_BINARY_PATH AND NINJA_BINARY_PATH)
    set (BASE_FREETYPE_TARGET ext_freetype_bootstrap)
endif()

ExternalProject_Add( ${BASE_FREETYPE_TARGET}
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    URL ${FREETYPE_URL}
    URL_HASH SHA256=${FREETYPE_SHA256}
    
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Publish-FreeType-dependencies.patch

    INSTALL_DIR ${PREFIX_ext_freetype}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DFT_REQUIRE_PNG=TRUE
        -DFT_DISABLE_BZIP2=TRUE
        -DFT_DISABLE_HARFBUZZ=TRUE
        -DBUILD_SHARED_LIBS=TRUE
        ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

ExternalProject_Get_property(${BASE_FREETYPE_TARGET} BINARY_DIR)

ExternalProject_Add_Step(${BASE_FREETYPE_TARGET} uninstall
    COMMAND ${CMAKE_COMMAND} -DPROJECT_BINARY_DIR=${BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/ProjectUninstallScript.cmake
    INDEPENDENT ON
    EXCLUDE_FROM_MAIN ON
)

ExternalProject_Add_StepTargets(${BASE_FREETYPE_TARGET} uninstall)



if(MESON_BINARY_PATH AND NINJA_BINARY_PATH)

    ExternalProject_Add(
        ext_harfbuzz
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/harfbuzz/harfbuzz/releases/download/4.1.0/harfbuzz-4.1.0.tar.xz
        URL_HASH SHA256=f7984ff4241d4d135f318a93aa902d910a170a8265b7eaf93b5d9a504eed40c8

        CONFIGURE_COMMAND meson <SOURCE_DIR> --prefix=${EXTPREFIX_harfbuzz} --libdir=lib

        BUILD_COMMAND meson compile -j${SUBMAKE_JOBS}

        INSTALL_COMMAND meson install

        DEPENDS ${BASE_FREETYPE_TARGET}
    )

    ExternalProject_Add( ext_freetype
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL ${FREETYPE_URL}
        URL_HASH SHA256=${FREETYPE_SHA256}

        INSTALL_DIR ${PREFIX_ext_freetype}

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
            -DFT_REQUIRE_PNG=TRUE
            -DFT_DISABLE_BZIP2=TRUE
            -DFT_REQUIRE_HARFBUZZ=TRUE
            -DBUILD_SHARED_LIBS=TRUE
            ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        DEPENDS ext_harfbuzz
    )

    ExternalProject_Add_StepDependencies(ext_freetype install ${BASE_FREETYPE_TARGET}-uninstall)

else()
    if (NOT MESON_BINARY_PATH)
        message(WARNING "Meson Build system was not found, harfbuzz will not be built. See https://mesonbuild.com")
    endif()
    if (NOT NINJA_BINARY_PATH)
        message(WARNING "Ninja Build system was not found, harfbuzz will not be built. See https://ninja-build.org/")
    endif()
endif()

