SET(PREFIX_ext_zlib "${EXTPREFIX}" )
if (NOT APPLE)
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/zlib-1.2.12.tar.gz
    URL_HASH SHA1=207ba741d387e2c1607104cf0bd8cff27deb2605

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_zlib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

if(WIN32 AND MINGW)
    # Workaround for zlib's library naming bug:
    # https://github.com/madler/zlib/issues/652

    ExternalProject_Add_Step(
        ext_zlib
        post_install
        COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_zlib}/lib/libzlib.dll.a ${PREFIX_ext_zlib}/lib/libz.dll.a
        DEPENDEES install
    )
endif()

else()
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/zlib-1.2.12.tar.gz
    URL_HASH SHA1=207ba741d387e2c1607104cf0bd8cff27deb2605

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_zlib}   ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
)
endif()
