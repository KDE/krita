SET(PREFIX_ext_openexr "${EXTPREFIX}" )
ExternalProject_Add( ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/openexr-2.2.0.tar.gz
        URL_MD5 b64e931c82aa3790329c21418373db4e
        
        INSTALL_DIR ${PREFIX_ext_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_openexr} -DILMBASE_PACKAGE_PREFIX=${PREFIX_ext_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF
            
        UPDATE_COMMAND ""
        ALWAYS 0
        DEPENDS ext_ilmbase
)

