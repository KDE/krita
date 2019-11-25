SET(PREFIX_ext_jpeg "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://api.github.com/repos/libjpeg-turbo/libjpeg-turbo/tarball/2.0.3
    URL_MD5 f927aa8dfbe47c3ba4d9f29d77b6805c
        
    INSTALL_DIR ${PREFIX_ext_jpeg}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DWITH_SIMD=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
elseif (ANDROID)
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://api.github.com/repos/libjpeg-turbo/libjpeg-turbo/tarball/2.0.3
    URL_MD5 f927aa8dfbe47c3ba4d9f29d77b6805c

    CMAKE_ARGS -DANDROID_ARM_MODE=arm -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DENABLE_SHARED=ON -DWITH_SIMD=OFF -DENABLE_STATIC=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_jpeg
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://api.github.com/repos/libjpeg-turbo/libjpeg-turbo/tarball/2.0.3
    URL_MD5 f927aa8dfbe47c3ba4d9f29d77b6805c
  
    INSTALL_DIR ${PREFIX_ext_jpeg}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpeg} -DWITH_SIMD=OFF _DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}


    UPDATE_COMMAND ""
)
endif()
