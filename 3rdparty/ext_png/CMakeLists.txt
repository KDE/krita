SET(PREFIX_ext_png "${EXTPREFIX}" )
ExternalProject_Add( ext_png
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/libpng-1.6.23.tar.xz
	URL_MD5 9b320a05ed4db1f3f0865c8a951fd9aa

	INSTALL_DIR ${PREFIX_ext_png}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_png} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
        
	UPDATE_COMMAND ""
	ALWAYS 0
	DEPENDS ext_zlib
)
