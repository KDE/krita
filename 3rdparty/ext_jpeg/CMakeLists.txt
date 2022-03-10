SET(PREFIX_ext_jpeg "${EXTPREFIX}" )
if (ANDROID)
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/libjpeg-turbo/2.1.3/libjpeg-turbo-2.1.3.tar.gz
    URL_HASH SHA256=467b310903832b033fe56cd37720d1b73a6a3bd0171dbf6ff0b620385f4f76d0

    CMAKE_ARGS -DANDROID_ARM_MODE=arm -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DENABLE_SHARED=ON -DWITH_SIMD=OFF -DENABLE_STATIC=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/libjpeg-turbo/2.1.3/libjpeg-turbo-2.1.3.tar.gz
    URL_HASH SHA256=467b310903832b033fe56cd37720d1b73a6a3bd0171dbf6ff0b620385f4f76d0
  
    INSTALL_DIR ${PREFIX_ext_jpeg}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DWITH_SIMD=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}


    UPDATE_COMMAND ""
)
endif()
