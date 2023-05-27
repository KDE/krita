SET(PREFIX_ext_jpeg "${EXTPREFIX}" )
if (ANDROID)
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/libjpeg-turbo/2.1.4/libjpeg-turbo-2.1.4.tar.gz
    URL_HASH SHA256=d3ed26a1131a13686dfca4935e520eb7c90ae76fbc45d98bb50a8dc86230342b

    CMAKE_ARGS -DANDROID_ARM_MODE=arm -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DENABLE_SHARED=ON -DWITH_SIMD=OFF -DENABLE_STATIC=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/libjpeg-turbo/2.1.4/libjpeg-turbo-2.1.4.tar.gz
    URL_HASH SHA256=d3ed26a1131a13686dfca4935e520eb7c90ae76fbc45d98bb50a8dc86230342b
  
    INSTALL_DIR ${PREFIX_ext_jpeg}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DWITH_SIMD=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}


    UPDATE_COMMAND ""
)
endif()
