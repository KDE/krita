SET(PREFIX_ext_giflib "${EXTPREFIX}" )
if (MINGW OR MSVC)
ExternalProject_Add(
    ext_giflib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/giflib-5.0.5.tar.gz
    URL_MD5 7c1c9434ce892d57a78c63cf15bbfbcc
    ATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/giflib-5.0.5-20130916.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_giflib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add(
    ext_giflib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/giflib-5.0.5.tar.gz
    URL_MD5 7c1c9434ce892d57a78c63cf15bbfbcc

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_giflib}   ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)
endif()
