SET(EXTPREFIX_eigen3 "${EXTPREFIX}" )
ExternalProject_Add( ext_eigen3
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    # eigen 3.3.4: bitbucket does weird things when downloading.
    URL http://files.kde.org/krita/build/dependencies/eigen-eigen-5a0156e40feb.tar.gz
    URL_MD5 1a47e78efe365a97de0c022d127607c3

    INSTALL_DIR ${EXTPREFIX_eigen3}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/dart.diff
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/no_tests.diff 
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_eigen3} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
