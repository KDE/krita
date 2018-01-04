SET(PREFIX_ext_png "${EXTPREFIX}" )
ExternalProject_Add( ext_png
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/libpng-1.6.34.tar.gz
	URL_MD5 42b801f2defaccb6b6cf1bf783ee1552

	INSTALL_DIR ${PREFIX_ext_png}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_png} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DPNG_TESTS=OFF
        
	UPDATE_COMMAND ""
	DEPENDS ext_zlib
)
