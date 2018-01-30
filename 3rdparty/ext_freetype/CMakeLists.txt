SET(PREFIX_ext_freetype "${EXTPREFIX}" )
if(APPLE OR UNIX)
ExternalProject_Add( ext_freetype
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/freetype-2.6.5.tar.gz
    URL_MD5 31b2276515d9ee1c7f37d9c9f4f3145a
        
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_freetype} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_freetype
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/freetype-2.6.5.tar.gz
    URL_MD5 31b2276515d9ee1c7f37d9c9f4f3145a
        
    INSTALL_DIR ${PREFIX_ext_freetype}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_freetype} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}        

    UPDATE_COMMAND ""
)
endif()
