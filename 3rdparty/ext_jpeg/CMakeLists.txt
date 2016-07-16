SET(PREFIX_ext_jpeg "${EXTPREFIX}" )
if (MSVC)
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/libjpeg-turbo-1.5.0.tar.gz
    URL_MD5 3fc5d9b6a8bce96161659ae7a9939257
        
    INSTALL_DIR ${PREFIX_ext_jpeg}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DWITH_SIMD=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
	ALWAYS 0
)
else()
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/libjpeg-turbo-1.5.0.tar.gz
    URL_MD5 3fc5d9b6a8bce96161659ae7a9939257
  
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_jpeg}  ${GLOBAL_AUTOMAKE_PROFILE} --without-simd
    BUILD_COMMAND make
    INSTALL_COMMAND make install      

    UPDATE_COMMAND ""
    ALWAYS 0
)
endif()
