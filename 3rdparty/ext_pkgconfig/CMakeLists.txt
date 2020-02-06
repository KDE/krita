SET(PREFIX_pkgconfig "${EXTPREFIX}" )

ExternalProject_Add( ext_pkgconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://pkgconfig.freedesktop.org/releases/pkg-config-0.29.2.tar.gz
    URL_MD5 f6e931e319531b736fadc017f470e68a

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_pkgconfig} ${GLOBAL_AUTOMAKE_PROFILE} --with-internal-glib
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
