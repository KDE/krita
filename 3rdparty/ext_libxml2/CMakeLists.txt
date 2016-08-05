SET(PREFIX_ext_libxml2 "${EXTPREFIX}" )
if (MSVC OR MINGW)
    ExternalProject_Add( ext_libxml2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/libxml2-sources-2.9.3.tar.gz
        URL_MD5 817ee2f5297e9763097074575903682d

        INSTALL_DIR ${PREFIX_ext_libxml2}
        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/libxml2-2.8.0-20110105.diff
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_libxml2} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
        ALWAYS 0
        DEPENDS ext_patch ext_iconv ext_zlib
    )
else (MSVC OR MINGW)
    ExternalProject_Add( ext_libxml2
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL http://files.kde.org/krita/build/dependencies/libxml2-sources-2.9.3.tar.gz
            URL_MD5 817ee2f5297e9763097074575903682d

            INSTALL_DIR ${PREFIX_ext_libxml2}
            CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_libxml2}   ${GLOBAL_AUTOMAKE_PROFILE} --with-python-install-dir=${PREFIX_ext_libxml2}
            BUILD_COMMAND make
            INSTALL_COMMAND make install

            UPDATE_COMMAND ""
            ALWAYS 0
            DEPENDS ext_iconv ext_zlib
    )
endif (MSVC OR MINGW)
