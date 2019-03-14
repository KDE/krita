SET(PREFIX_ext_openssl "${EXTPREFIX}" )

ExternalProject_Add( ext_openssl
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.openssl.org/source/openssl-1.1.1b.tar.gz
    URL_HASH SHA1=e9710abf5e95c48ebf47991b10cbb48c09dae102

    CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${PREFIX_ext_openssl} --openssldir=ssl

    BUILD_COMMAND make depend COMMAND make
    INSTALL_COMMAND make install

    INSTALL_DIR ${PREFIX_ext_openssl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_fftw3} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)