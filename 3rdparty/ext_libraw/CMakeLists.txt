SET(PREFIX_ext_libraw "${EXTPREFIX}" )
ExternalProject_Add( ext_libraw
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/LibRaw-0.18.6.tar.gz
        URL_MD5 62459e4808eb730ff117b1710dc43d6d

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libraw.diff
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_libraw} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    	DEPENDS ext_lcms2 ext_jpeg
)

