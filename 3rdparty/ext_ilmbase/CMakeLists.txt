SET(PREFIX_ext_ilmbase "${EXTPREFIX}" )
ExternalProject_Add( ext_ilmbase
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/ilmbase-2.2.0.tar.gz
        URL_MD5 b540db502c5fa42078249f43d18a4652

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_ilmbase} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        ALWAYS 0
)
