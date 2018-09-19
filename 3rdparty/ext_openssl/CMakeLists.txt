SET(PREFIX_ext_openssl "${EXTPREFIX}" )
ExternalProject_Add( ext_openssl
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.openssl.org/source/openssl-1.1.1.tar.gz
        URL_MD5 7079eb017429e0ffb9efb42bf80ccb21

        CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${PREFIX_ext_openssl} --openssldir=${PREFIX_ext_openssl}
        BUILD_COMMAND make
        INSTALL_COMMAND make install

        INSTALL_DIR ${PREFIX_ext_openssl}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_fftw3} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
)
