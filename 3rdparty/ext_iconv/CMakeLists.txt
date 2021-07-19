SET(PREFIX_ext_iconv "${EXTPREFIX}" )
if (WIN32)
ExternalProject_Add( ext_iconv
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://github.com/win-iconv/win-iconv/archive/v0.0.8.tar.gz
	URL_MD5 e4574721c2d9667af32283175c828833
	
    INSTALL_DIR ${PREFIX_ext_iconv}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_iconv} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

	UPDATE_COMMAND ""
)
else ()
ExternalProject_Add( ext_iconv
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.16.tar.gz
	URL_MD5 7d2a800b952942bb2880efb00cfd524c
	
    INSTALL_DIR ${PREFIX_ext_iconv}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/iconv.diff
	CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_iconv}  ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
	
    UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
)

endif ()
