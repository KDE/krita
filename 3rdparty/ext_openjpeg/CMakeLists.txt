SET(PREFIX_ext_openjpeg "${EXTPREFIX}" )
ExternalProject_Add( ext_openjpeg
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://files.kde.org/krita/build/dependencies/openjpeg-version.2.3.1.tar.gz
	URL_MD5 3b9941dc7a52f0376694adb15a72903f
	
    INSTALL_DIR ${PREFIX_ext_openjpeg}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_openjpeg} -DOPENJPEG_INSTALL_INCLUDE_DIR=include/ -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
	
    UPDATE_COMMAND ""
	ALWAYS 0
    DEPENDS ext_lcms2
)
