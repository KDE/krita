SET(PREFIX_ext_freetype "${EXTPREFIX}" )
ExternalProject_Add( ext_freetype
	DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
	URL http://files.kde.org/krita/build/dependencies/freetype-2.6.tar.gz
    URL_MD5 1d733ea6c1b7b3df38169fbdbec47d2b
        
    INSTALL_DIR ${PREFIX_ext_freetype}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_freetype} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}        

    UPDATE_COMMAND ""
	ALWAYS 0
)
