SET(PREFIX_ext_quazip "${EXTPREFIX}" )
ExternalProject_Add( ext_quazip
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/stachenov/quazip/archive/0.7.6.zip
    URL_MD5 a3335649c34053385d8390dd1a6f1ca4
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/find_quazip.diff
    INSTALL_DIR ${PREFIX_ext_quazip}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_quazip} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} 
    
    UPDATE_COMMAND ""
    DEPENDS ext_zlib ext_qt
)
