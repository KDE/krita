SET(EXTPREFIX_tiff "${EXTPREFIX}" )
if (MSVC)
ExternalProject_Add( ext_tiff
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/tiff-4.0.3.tar.gz
    URL_MD5 051c1068e6a0627f461948c365290410
        
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/tiff-4.0.3-20130124.diff
    INSTALL_DIR ${EXTPREFIX_tiff}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_tiff} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_patch ext_zlib ext_jpeg
)
else (MSVC)
ExternalProject_Add( ext_tiff
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/tiff-4.0.3.tar.gz
	URL_MD5 051c1068e6a0627f461948c365290410

    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_tiff}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
	
    UPDATE_COMMAND ""
	ALWAYS 0
	DEPENDS ext_zlib ext_jpeg
)
endif (MSVC)
