SET(PREFIX_pkgconfig "${EXTPREFIX}" )

if (NOT WIN32)
ExternalProject_Add( ext_pkgconfig
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://pkgconfig.freedesktop.org/releases/pkg-config-0.29.2.tar.gz
    URL_MD5 f6e931e319531b736fadc017f470e68a

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_pkgconfig} ${GLOBAL_AUTOMAKE_PROFILE} --with-internal-glib

)
else()

ExternalProject_Add( ext_pkgconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/pkgconf/pkgconf.git
    GIT_TAG pkgconf-1.9.3

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
        ${MESON_BINARY_PATH} setup <BINARY_DIR> <SOURCE_DIR>
            --prefix=${PREFIX_pkgconfig}
            --libdir=lib
            -Dbuildtype=$<IF:$<CONFIG:Debug>,debug,debugoptimized>
            -Dtests=false
            ${EXTRA_MESON_FLAGS}

    BUILD_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} compile -C <BINARY_DIR> -j${SUBMAKE_JOBS}

    INSTALL_COMMAND ${CMAKE_COMMAND} -E env
        PYTHONPATH=${_krita_pythonpath}
        ${MESON_BINARY_PATH} install -C <BINARY_DIR>

    UPDATE_COMMAND ""

    DEPENDS ${MESON_DEP}
)
endif()
