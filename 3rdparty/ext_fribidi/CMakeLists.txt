SET(EXTPREFIX_fribidi "${EXTPREFIX}" )


kis_ExternalProject_Add_with_separate_builds_apple(
    ext_fribidi
    MESON

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/fribidi/fribidi/releases/download/v1.0.11/fribidi-1.0.11.tar.xz
    URL_HASH SHA256=30f93e9c63ee627d1a2cedcf59ac34d45bf30240982f99e44c6e015466b4e73d

    CONFIGURE_ARGS --prefix=${EXTPREFIX_fribidi}
            ${EXTRA_MESON_FLAGS}

    DEPENDS ${MESON_DEP} ${PKG_CONFIG_DEP}
)
