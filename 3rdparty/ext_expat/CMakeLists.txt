SET(PREFIX_ext_expat "${EXTPREFIX}" )

if (WIN32)
ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://github.com/libexpat/libexpat/releases/download/R_2_2_9/expat-2.2.9.tar.gz
	URL_MD5 077b953cc38df8fed78e92841cc35443

	PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/expat-2.2.9-b9bd355.diff
	INSTALL_DIR ${PREFIX_ext_expat}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_expat} -DBUILD_tests=OFF -DBUILD_examples=OFF -DBUILD_tools=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
	UPDATE_COMMAND ""

	DEPENDS ext_patch
)

elseif (ANDROID)
ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://github.com/libexpat/libexpat/releases/download/R_2_2_9/expat-2.2.9.tar.gz
	URL_MD5 077b953cc38df8fed78e92841cc35443

	INSTALL_DIR ${PREFIX_ext_expat}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_expat} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} <SOURCE_DIR>/expat
	BUILD_COMMAND make
	INSTALL_COMMAND make install

	UPDATE_COMMAND ""
)

else()

ExternalProject_Add( ext_expat
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://github.com/libexpat/libexpat/releases/download/R_2_2_9/expat-2.2.9.tar.gz
	URL_MD5 077b953cc38df8fed78e92841cc35443

	INSTALL_DIR ${PREFIX_ext_expat}
	CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_expat} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} <SOURCE_DIR>
	BUILD_COMMAND make
	INSTALL_COMMAND make install

	UPDATE_COMMAND ""

)

endif()
