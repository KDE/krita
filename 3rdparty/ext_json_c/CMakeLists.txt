SET(PREFIX_ext_json_c "${EXTPREFIX}" )
ExternalProject_Add( ext_json_c
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/json-c/json-c/archive/refs/tags/json-c-0.16-20220414.tar.gz
    URL_HASH SHA256=3ecaeedffd99a60b1262819f9e60d7d983844073abc74e495cb822b251904185

    INSTALL_DIR ${PREFIX_ext_json_c}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_json_c} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
