SET(PREFIX_ext_freetype "${EXTPREFIX}" )
if(APPLE OR UNIX)
ExternalProject_Add( ext_freetype
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://download.savannah.gnu.org/releases/freetype/freetype-2.10.1.tar.gz
    URL_MD5 c50a3c9e5e62bdc938a6e1598a782947
        
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_freetype} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_freetype
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL https://download.savannah.gnu.org/releases/freetype/freetype-2.10.1.tar.gz
    URL_MD5 c50a3c9e5e62bdc938a6e1598a782947
        
    INSTALL_DIR ${PREFIX_ext_freetype}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_freetype} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}        

    UPDATE_COMMAND ""
)
endif()
