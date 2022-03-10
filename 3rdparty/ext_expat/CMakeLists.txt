SET(PREFIX_ext_expat "${EXTPREFIX}" )

ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://github.com/libexpat/libexpat/releases/download/R_2_4_7/expat-2.4.7.tar.xz
	URL_HASH SHA256=9875621085300591f1e64c18fd3da3a0eeca4a74f884b9abac2758ad1bd07a7d

	INSTALL_DIR ${PREFIX_ext_expat}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_expat} -DBUILD_tests=OFF -DBUILD_examples=OFF -DBUILD_tools=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} <SOURCE_DIR>/expat
	UPDATE_COMMAND ""
)
