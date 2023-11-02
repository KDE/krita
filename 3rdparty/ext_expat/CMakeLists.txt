SET(PREFIX_ext_expat "${EXTPREFIX}" )

ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libexpat/libexpat/releases/download/R_2_5_0/expat-2.5.0.tar.gz
    URL_HASH SHA256=6b902ab103843592be5e99504f846ec109c1abb692e85347587f237a4ffa1033

	INSTALL_DIR ${PREFIX_ext_expat}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_expat} -DBUILD_tests=OFF -DBUILD_examples=OFF -DBUILD_tools=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} <SOURCE_DIR>/expat
	UPDATE_COMMAND ""
)
