SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 


kis_ExternalProject_Add_with_separate_builds_apple(
    ext_raqm
    MESON
    URL https://github.com/HOST-Oman/libraqm/releases/download/v0.10.0/raqm-0.10.0.tar.xz
    URL_HASH SHA256=5eb7de71b3d1dca34103860f3e8c55ebc184a3dbdfdd573c21a258e46982f1e6

    CONFIGURE_ARGS --prefix=${EXTPREFIX_raqm}
            --default-library=shared
            ${EXTRA_MESON_FLAGS}

    UPDATE_COMMAND ""

    DEPENDS ext_freetype ext_harfbuzz ext_fribidi ${MESON_DEP} ${PKG_CONFIG_DEP}
)
