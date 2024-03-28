SET(PREFIX_ext_iconv "${EXTPREFIX}" )
if (WIN32)
ExternalProject_Add( ext_iconv
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	DOWNLOAD_NAME win-iconv-v0.0.8.tar.gz
	URL https://github.com/win-iconv/win-iconv/archive/v0.0.8.tar.gz
	URL_MD5 e4574721c2d9667af32283175c828833

    INSTALL_DIR ${PREFIX_ext_iconv}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_iconv} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

	UPDATE_COMMAND ""
)
else ()
ExternalProject_Add( ext_iconv
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://ftp.gnu.org/gnu/libiconv/libiconv-1.17.tar.gz
	URL_HASH SHA256=8f74213b56238c85a50a5329f77e06198771e70dd9a739779f4c02f65d971313

	INSTALL_DIR ${PREFIX_ext_iconv}
	CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_iconv}  ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
)

endif ()
