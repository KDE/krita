SET(PREFIX_ext_quazip "${EXTPREFIX}" )
ExternalProject_Add( ext_quazip
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/stachenov/quazip/archive/v0.9.1.tar.gz
    URL_HASH SHA256=5d36b745cb94da440432690050e6db45b99b477cfe9bc3b82fd1a9d36fff95f5

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/liblocation.diff

    INSTALL_DIR ${PREFIX_ext_quazip}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_quazip} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${ZLIB_DEP}
)

