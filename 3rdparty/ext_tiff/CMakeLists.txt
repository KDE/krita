SET(EXTPREFIX_tiff "${EXTPREFIX}" )
ExternalProject_Add( ext_tiff
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/tiff-4.0.9.tar.gz
	URL_MD5 54bad211279cc93eb4fca31ba9bfdc79

    INSTALL_DIR ${EXTPREFIX_tiff}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_tiff} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -Dlzma=OFF
	
    UPDATE_COMMAND ""
)
