SET(EXTPREFIX_icu "${EXTPREFIX}" )

ExternalProject_Add(
    ext_icu_meson
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://wrapdb.mesonbuild.com/v2/icu_73.2-2/get_patch
    URL_HASH SHA256=218a5f20b58b6b2372e636c2eb2d611a898fdc11be17d6c4f35a3cd54d472010

    DOWNLOAD_NAME "icu_73.2-2_patch.zip"

    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""

    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson"
)

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_icu
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/unicode-org/icu/releases/download/release-73-2/icu4c-73_2-src.tgz
    URL_HASH SHA256=818a80712ed3caacd9b652305e01afc7fa167e6f2e94996da44b90c2ab604ce1

    PATCH_COMMAND ${CMAKE_COMMAND} -E echo Deploying Meson wrap
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_BINARY_DIR}/meson
            <SOURCE_DIR>
        COMMAND ${CMAKE_COMMAND} -E echo Deploying patches
        COMMAND ${PATCH_COMMAND} -p2 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-MinGW-support-from-MSYS.patch
        COMMAND ${PATCH_COMMAND} -p2 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Windows-enable-C-code-generation-with-overridden-ent.patch
        COMMAND ${PATCH_COMMAND} -p2 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-toolutil-Fix-MASM-generation-for-x86-64-and-ARM64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-meson-Use-NASM-to-work-around-MSVC-compiler-performa.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0005-meson-Work-around-Meson-linking-bug-when-there-s-no-.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0006-meson-Enable-icudata-for-MSVC.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0007-meson-Work-around-windeployqt-expecting-no-ICU-prefi.patch

    CONFIGURE_ARGS --prefix=${EXTPREFIX_icu} ${EXTRA_MESON_FLAGS}

    UPDATE_COMMAND ""

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP} ext_icu_meson
)
