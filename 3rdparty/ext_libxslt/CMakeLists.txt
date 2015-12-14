SET(PREFIX_ext_libxslt "${EXTPREFIX}" )
if (MSVC)
ExternalProject_Add( ext_libxslt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/libxslt-1.1.26.tar.gz
        URL_MD5 e61d0364a30146aaa3001296f853b2b9

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libxslt-1.1.26-20101102.diff
        INSTALL_DIR ${PREFIX_ext_libxslt}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_libxslt} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        ALWAYS 0
        DEPENDS ext_patch ext_libxml2
)
else (MSVC)
ExternalProject_Add( ext_libxslt
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/libxslt-1.1.26.tar.gz
        URL_MD5 e61d0364a30146aaa3001296f853b2b9

        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_libxslt}  ${GLOBAL_AUTOMAKE_PROFILE}
        BUILD_COMMAND make
        INSTALL_COMMAND make install

        UPDATE_COMMAND ""
        ALWAYS 0
        DEPENDS ext_libxml2
)
endif (MSVC)
