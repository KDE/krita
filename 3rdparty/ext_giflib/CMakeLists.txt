SET(PREFIX_ext_giflib "${EXTPREFIX}" )
if (MINGW )
ExternalProject_Add(
    ext_giflib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/giflib-5.1.4.tar.bz2
    URL_MD5 2c171ced93c0e83bb09e6ccad8e3ba2b

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/giflib-5.1.4-20180220.diff
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_giflib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DBUILD_utils=OFF

    UPDATE_COMMAND ""
)
elseif (ANDROID)
ExternalProject_Add(
    ext_giflib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/giflib-5.1.4.tar.bz2
    URL_MD5 2c171ced93c0e83bb09e6ccad8e3ba2b

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/android-cmake.patch
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_giflib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DBUILD_utils=OFF

    UPDATE_COMMAND ""
)

else()
ExternalProject_Add(
    ext_giflib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/giflib-5.1.4.tar.bz2
    URL_MD5 2c171ced93c0e83bb09e6ccad8e3ba2b

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_giflib}   ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)
endif()
