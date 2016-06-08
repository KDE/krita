SET(PREFIX_ext_openjpeg "${EXTPREFIX}" )
ExternalProject_Add( ext_openjpeg
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/openjpeg-version.2.1.tar.gz
	URL_MD5 3e1c451c087f8462955426da38aa3b3d
	
    INSTALL_DIR ${PREFIX_ext_openjpeg}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_openjpeg} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
	
    UPDATE_COMMAND ""
	ALWAYS 0
    DEPENDS ext_png ext_tiff ext_lcms2 ext_zlib
)

