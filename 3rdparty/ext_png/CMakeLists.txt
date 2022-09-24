SET(PREFIX_ext_png "${EXTPREFIX}" )
ExternalProject_Add( ext_png
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/libpng/libpng16/1.6.38/libpng-1.6.38.tar.xz
    URL_HASH SHA256=b3683e8b8111ebf6f1ac004ebb6b0c975cd310ec469d98364388e9cedbfa68be

	INSTALL_DIR ${PREFIX_ext_png}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_png} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DPNG_TESTS=OFF

	UPDATE_COMMAND ""
)
