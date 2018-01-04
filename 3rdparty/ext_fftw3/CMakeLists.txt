SET(PREFIX_ext_fftw3 "${EXTPREFIX}" )
ExternalProject_Add( ext_fftw3
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
     	URL http://www.fftw.org/fftw-3.3.7.tar.gz
        URL_MD5 0d5915d7d39b3253c1cc05030d79ac47
        INSTALL_DIR ${PREFIX_ext_fftw3}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_fftw3} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}  
        INSTALL_COMMAND make install
        UPDATE_COMMAND ""
)
