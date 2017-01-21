SET(PREFIX_ext_pthreads "${EXTPREFIX}" )
ExternalProject_Add( ext_pthreads
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/pthreads-w32-2-9-1-release.tar.gz
    URL_MD5 36ba827d6aa0fa9f9ae740a35626e2e3

    INSTALL_DIR ${PREFIX_ext_pthreads}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/pthreads-w32-2-8-0-release-20110729.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_pthreads} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ext_patch
)
