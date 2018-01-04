ExternalProject_Add( ext_tiff
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/libtiff-Release-v4-0-9-020bd2fd3b4235b2175dafa9633d2b191c50e7cd.tar.gz
	URL_MD5 34323035fa1acf88ce027958457d6404

    INSTALL_DIR ${EXTPREFIX_tiff}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_tiff} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    BUILD_COMMAND make
    INSTALL_COMMAND make install
	
    UPDATE_COMMAND ""
	DEPENDS ext_zlib ext_jpeg
)
