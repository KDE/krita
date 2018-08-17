SET(PREFIX_ext_iconv "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add( ext_iconv
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/win-iconv-0.0.6.tar.bz2
	URL_MD5 1e97ed4d9e7379ff0ee22077256e8c58
	
    INSTALL_DIR ${PREFIX_ext_iconv}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_iconv} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

	UPDATE_COMMAND ""
)
else ()
ExternalProject_Add( ext_iconv
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/libiconv-1.14.tar.gz
	URL_MD5 e34509b1623cec449dfeb73d7ce9c6c6
	
    INSTALL_DIR ${PREFIX_ext_iconv}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/iconv.diff
	CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_iconv}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
	
    UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
)

endif ()
