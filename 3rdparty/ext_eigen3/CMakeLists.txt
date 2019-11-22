SET(EXTPREFIX_eigen3 "${EXTPREFIX}" )
ExternalProject_Add( ext_eigen3
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    # eigen 3.3.4: bitbucket does weird things when downloading.
    URL http://bitbucket.org/eigen/eigen/get/3.3.7.tar.bz2
    URL_MD5 05b1f7511c93980c385ebe11bd3c93fa

    INSTALL_DIR ${EXTPREFIX_eigen3}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/dart.diff
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/no_tests.diff 
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_eigen3} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
