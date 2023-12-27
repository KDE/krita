SET(PREFIX_ext_expat "${EXTPREFIX}" )

ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libexpat/libexpat/releases/download/R_2_4_8/expat-2.4.8.tar.xz
    URL_HASH SHA256=f79b8f904b749e3e0d20afeadecf8249c55b2e32d4ebb089ae378df479dcaf25

	INSTALL_DIR ${PREFIX_ext_expat}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_expat} -DBUILD_tests=OFF -DBUILD_examples=OFF -DBUILD_tools=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} <SOURCE_DIR>/expat
	UPDATE_COMMAND ""
)
