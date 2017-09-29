SET(PREFIX_ext_freetype "${EXTPREFIX}" )
ExternalProject_Add( ext_freetype
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/freetype-2.6.5.tar.gz
    URL_MD5 31b2276515d9ee1c7f37d9c9f4f3145a
        
    INSTALL_DIR ${PREFIX_ext_freetype}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_freetype} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}        

    UPDATE_COMMAND ""
)
