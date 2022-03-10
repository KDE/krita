SET(EXTPREFIX_tiff "${EXTPREFIX}" )
ExternalProject_Add( ext_tiff
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.osgeo.org/libtiff/tiff-4.3.0.tar.gz
    URL_HASH SHA256=0e46e5acb087ce7d1ac53cf4f56a09b221537fc86dfc5daaad1c2e89e1b37ac8

    PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Enable-writing-Photoshop-blobs.patch

    INSTALL_DIR ${EXTPREFIX_tiff}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_tiff} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
