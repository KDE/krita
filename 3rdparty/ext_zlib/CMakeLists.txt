SET(PREFIX_ext_zlib "${EXTPREFIX}" )
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/zlib-1.2.11.tar.gz
    URL_MD5 1c9f62f0778697a09d36121ead88e08e

    INSTALL_DIR ${PREFIX_ext_zlib}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_zlib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
