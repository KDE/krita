SET(PREFIX_ext_libraw "${EXTPREFIX}" )
ExternalProject_Add( ext_libraw
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/LibRaw-0.17.2.tar.gz
        URL_MD5 456626300777209def1ea784910f326a

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libraw.diff
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_libraw} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        ALWAYS 0
    	DEPENDS ext_lcms2 ext_jpeg
)

