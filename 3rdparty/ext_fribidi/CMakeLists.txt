SET(EXTPREFIX_fribidi "${EXTPREFIX}" )


kis_ExternalProject_Add_with_separate_builds_apple(
    ext_fribidi
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/fribidi/fribidi/releases/download/v1.0.13/fribidi-1.0.13.tar.xz
    URL_HASH SHA256=7fa16c80c81bd622f7b198d31356da139cc318a63fc7761217af4130903f54a2

    CONFIGURE_ARGS --prefix=${EXTPREFIX_fribidi}
            ${EXTRA_MESON_FLAGS}

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP}
)
