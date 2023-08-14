SET(EXTPREFIX_raqm "${EXTPREFIX}" ) 


kis_ExternalProject_Add_with_separate_builds_apple(
    ext_raqm
    MESON
    URL https://github.com/HOST-Oman/libraqm/releases/download/v0.10.1/raqm-0.10.1.tar.xz
    URL_HASH SHA256=4d76a358358d67c5945684f2f10b3b08fb80e924371bf3ebf8b15cd2e321d05d

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-arbitrary-run-break-function.patch

    CONFIGURE_ARGS --prefix=${EXTPREFIX_raqm}
            --default-library=shared
            ${EXTRA_MESON_FLAGS}

    UPDATE_COMMAND ""

    DEPENDS ext_freetype ext_harfbuzz ext_fribidi ${MESON_DEP} ${PKG_CONFIG_DEP}
)
