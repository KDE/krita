SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 

ExternalProject_Add(
    ext_raqm
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/therahedwig/libraqm/archive/793d7a5e89f3a2bdda8d9236d24294e48dc2e354.zip
    URL_HASH SHA256=576e63009a26d613aab8dd3bd216be43a49979d26aa99cb186a0008a9f3af252
    
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/fix_utf_16.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${EXTPREFIX_raqm}
            --libdir=lib
            --default-library=shared
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            ${EXTRA_MESON_FLAGS}

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    DEPENDS ext_freetype ext_harfbuzz ext_fribidi ${MESON_DEP} ${PKG_CONFIG_DEP}
)
