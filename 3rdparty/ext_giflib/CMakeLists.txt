SET(PREFIX_ext_giflib "${EXTPREFIX}" )
ExternalProject_Add(
    ext_giflib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/giflib/giflib-5.2.1.tar.gz
    URL_MD5 6f03aee4ebe54ac2cc1ab3e4b0a049e5

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/giflib-cmake-5.2.1.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_giflib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DBUILD_utils=OFF

    UPDATE_COMMAND ""
)
