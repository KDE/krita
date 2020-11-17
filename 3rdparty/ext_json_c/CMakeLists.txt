SET(PREFIX_ext_json_c "${EXTPREFIX}" )
ExternalProject_Add( ext_json_c
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/json-c/json-c/archive/json-c-0.15-20200726.tar.gz
    URL_MD5 
        
    INSTALL_DIR ${PREFIX_ext_json_c}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_json_c} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
