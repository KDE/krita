SET(EXTPREFIX_xsimd "${EXTPREFIX}" )

ExternalProject_Add( ext_xsimd
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/xtensor-stack/xsimd/archive/8.0.5/xsimd-8.0.5.tar.gz
    URL_HASH SHA512=165ca307bcaccc226dd518562a9269aa9fc44c70fab299ea0827d7a0a9cf58ec493b381fcf2c4d9d1f6edfda8fa309643c63b17fb20d9de78a4be19eb70e30b3

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_xsimd} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
