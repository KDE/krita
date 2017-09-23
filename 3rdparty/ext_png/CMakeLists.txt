SET(PREFIX_ext_png "${EXTPREFIX}" )
ExternalProject_Add( ext_png
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/libpng-1.6.23.tar.gz
	URL_MD5 3a8fb380b0fb39cb69efe47901917e38

	INSTALL_DIR ${PREFIX_ext_png}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_png} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DPNG_TESTS=OFF
        
	UPDATE_COMMAND ""
	DEPENDS ext_zlib
)
