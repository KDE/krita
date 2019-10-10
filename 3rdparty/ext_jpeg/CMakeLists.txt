SET(PREFIX_ext_jpeg "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/2.0.2.tar.gz
    URL_MD5 8badea1368f489ac4c520f01b1bc045b
        
    INSTALL_DIR ${PREFIX_ext_jpeg}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DWITH_SIMD=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
elseif (ANDROID)
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/2.0.2.tar.gz
    URL_MD5 8badea1368f489ac4c520f01b1bc045b

    CMAKE_ARGS -DANDROID_ARM_MODE=arm -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DENABLE_SHARED=ON -DWITH_SIMD=OFF -DENABLE_STATIC=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libjpeg-turbo/libjpeg-turbo/archive/2.0.2.tar.gz
    URL_MD5 8badea1368f489ac4c520f01b1bc045b
  
    INSTALL_DIR ${PREFIX_ext_jpeg}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DWITH_SIMD=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}


    UPDATE_COMMAND ""
)
endif()
