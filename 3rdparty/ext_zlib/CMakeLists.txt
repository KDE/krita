SET(PREFIX_ext_zlib "${EXTPREFIX}" )
if (NOT APPLE)
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/zlib-1.2.11.tar.gz
    URL_MD5 1c9f62f0778697a09d36121ead88e08e

    INSTALL_DIR ${PREFIX_ext_zlib}

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/zlib-1.2.11.tar.gz
    URL_MD5 1c9f62f0778697a09d36121ead88e08e

    INSTALL_DIR ${PREFIX_ext_zlib}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_zlib}   ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)
endif()
