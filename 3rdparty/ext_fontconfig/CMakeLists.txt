SET(PREFIX_ext_fontconfig "${EXTPREFIX}" )

ExternalProject_Add( ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.14.0.tar.gz
    URL_HASH SHA256=b8f607d556e8257da2f3616b4d704be30fd73bd71e367355ca78963f9a7f0434

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-fontconfig-Do-not-create-symlinks-microsoft-23735.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-fontconfig-Enable-tool-CLI-option-handling-24553.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${PREFIX_ext_fontconfig}
            --libdir=lib -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Ddoc=disabled
            -Dcache-build=disabled
            -Dtests=disabled
            ${EXTRA_MESON_FLAGS}

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    DEPENDS ext_freetype ${GETTEXT_DEP} ${MESON_DEP} ${PKG_CONFIG_DEP}
)
