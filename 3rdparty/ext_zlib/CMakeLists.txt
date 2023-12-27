SET(PREFIX_ext_zlib "${EXTPREFIX}" )
if (NOT APPLE)
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.gz
    URL_HASH SHA1=55eaa84906f31ac20d725aa26cd20839196b6ba6

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Prevent-invalid-inclusions-when-HAVE_-is-set-to-0.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_zlib} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

if(WIN32)
    # Workaround for zlib's library naming bug:
    # https://github.com/madler/zlib/issues/652
    if (MINGW)
        ExternalProject_Add_Step(
            ext_zlib
            post_install
            COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_zlib}/lib/libzlib.dll.a ${PREFIX_ext_zlib}/lib/libz.dll.a
            DEPENDEES install
        )
    elseif(MSVC)
        ExternalProject_Add_Step(
            ext_zlib
            post_install
            COMMAND ${CMAKE_COMMAND} -E copy ${PREFIX_ext_zlib}/lib/zlib.lib ${PREFIX_ext_zlib}/lib/z.lib
            DEPENDEES install
        )
    endif()
endif()

else()
ExternalProject_Add(
    ext_zlib
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.gz
    URL_HASH SHA1=55eaa84906f31ac20d725aa26cd20839196b6ba6

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_zlib} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
endif()
