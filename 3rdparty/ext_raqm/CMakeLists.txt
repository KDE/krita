SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 

ExternalProject_Add(
    ext_raqm
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/HOST-Oman/libraqm/releases/download/v0.10.0/raqm-0.10.0.tar.xz
    URL_HASH SHA256=5eb7de71b3d1dca34103860f3e8c55ebc184a3dbdfdd573c21a258e46982f1e6

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

    UPDATE_COMMAND ""

    DEPENDS ext_freetype ext_harfbuzz ext_fribidi ${MESON_DEP} ${PKG_CONFIG_DEP}
)
