SET(PREFIX_ext_poppler "${EXTPREFIX}" )
ExternalProject_Add( ext_poppler
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/poppler-0.38.0.tar.gz
	URL_MD5 4554b04eedfd56c3a65be9f108e306f5
	
	INSTALL_DIR ${PREFIX_ext_poppler}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_poppler} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
	
	DEPENDS ext_freetype ext_jpeg ext_png ext_tiff ext_zlib ext_lcms2
        UPDATE_COMMAND ""
	ALWAYS 0
)
