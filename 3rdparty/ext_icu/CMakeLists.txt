SET(EXTPREFIX_icu "${EXTPREFIX}" )

if(NOT MESON_BINARY_PATH)
    set(MESON_BINARY_PATH ${EXTPREFIX_icu}/bin/meson)
endif()

if(NOT PKG_CONFIG_EXECUTABLE)
    set(PKG_CONFIG_EXECUTABLE ${EXTPREFIX_icu}/bin/pkgconf)
endif()

ExternalProject_Add(
    ext_icu_meson
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://wrapdb.mesonbuild.com/v2/icu_72.1-1/get_patch
    URL_HASH SHA256=9de6c52ca6a364264adeb2c7857dda3e94c020b9d4075c58c039d833239ebe7f

    DOWNLOAD_NAME "icu_72.1-1_patch.zip"

    BUILD_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""

    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson"
)

# Patch 0001 is the Meson wrap, the official source of which is above
ExternalProject_Add(
    ext_icu
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/unicode-org/icu/releases/download/release-72-1/icu4c-72_1-src.tgz
    URL_HASH SHA256=a2d2d38217092a7ed56635e34467f92f976b370e20182ad325edea6681a71d68

    PATCH_COMMAND ${CMAKE_COMMAND} -E echo Deploying Meson wrap
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_BINARY_DIR}/meson
            <SOURCE_DIR>
        COMMAND ${CMAKE_COMMAND} -E echo Deploying patches
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-MinGW-support-from-MSYS.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Windows-enable-C-code-generation-with-overridden-ent.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-toolutil-Fix-crash-when-trying-to-generate-MinGW-ass.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0005-meson-Fix-libraries-naming.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0006-meson-Support-resource-files-in-Windows.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0007-meson-Add-soversion-to-prevent-system-lookup-issues.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0008-meson-Make-library-type-and-linking-consistent-for-M.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0009-meson-Enable-using-libicu-wrap-as-a-standalone-proje.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0010-meson-Add-missing-defines-for-stubdata.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0011-meson-Support-building-libicudata.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0012-meson-fix-compatibility-with-0.49.0.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0013-toolutil-Fix-MASM-generation-for-x86-64-and-ARM64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0014-meson-add-commented-out-support-for-MASM.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0015-meson-Fix-missing-header-installation.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0016-meson-Fix-missing-icudata-library-export-for-MSVC.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0017-meson-Add-missing-metadata-for-layoutex.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0018-meson-Add-pkg-config-entries.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        CC="${CMAKE_C_COMPILER}"
        CXX="${CMAKE_CXX_COMPILER}"
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG=${PKG_CONFIG_EXECUTABLE}
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${EXTPREFIX_icu}
            --libdir=lib
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Dc_args=${_security_c_flags}
            -Dcpp_args=${_security_cxx_flags}
            -Dc_link_args=${_security_exe_linker_flags}
            -Dcpp_link_args=${_security_exe_linker_flags}
            -Dpkg_config_path=${EXTPREFIX_icu}/share/pkgconfig,${EXTPREFIX_icu}/lib/pkgconfig

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP} ext_icu_meson
)
