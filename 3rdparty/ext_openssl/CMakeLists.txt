SET(PREFIX_ext_openssl "${EXTPREFIX}" )

if (WIN32)

    # Use the binaries from curl for Windows (https://github.com/curl/curl-for-win).
    # The original source URLs are:
    # - https://curl.haxx.se/windows/dl-7.64.1/openssl-1.1.1b-win64-mingw.zip
    # - https://curl.haxx.se/windows/dl-7.64.1/openssl-1.1.1b-win32-mingw.zip
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_openssl
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://files.kde.org/krita/build/dependencies/openssl-1.1.1b-win64-mingw.zip
            URL_MD5 2dd7041cdcdfb44ad395fd6d4db216a2

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying openssl 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${PREFIX_ext_openssl}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libcrypto.dll.a ${PREFIX_ext_openssl}/lib
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libssl.dll.a ${PREFIX_ext_openssl}/lib
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/libcrypto-1_1-x64.dll ${PREFIX_ext_openssl}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/libssl-1_1-x64.dll ${PREFIX_ext_openssl}/bin
            UPDATE_COMMAND ""
        )
    else ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_openssl
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://files.kde.org/krita/build/dependencies/openssl-1.1.1b-win32-mingw.zip
            URL_MD5 24021021fb27c357f0728844cfce3927

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying openssl 32-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${PREFIX_ext_openssl}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libcrypto.dll.a ${PREFIX_ext_openssl}/lib
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libssl.dll.a ${PREFIX_ext_openssl}/lib
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/libcrypto-1_1.dll ${PREFIX_ext_openssl}/bin
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/libssl-1_1.dll ${PREFIX_ext_openssl}/bin
            UPDATE_COMMAND ""
        )
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

else (WIN32)

    ExternalProject_Add( ext_openssl
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.openssl.org/source/openssl-1.1.1d.tar.gz
        URL_HASH SHA1=056057782325134b76d1931c48f2c7e6595d7ef4

        CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${PREFIX_ext_openssl} --openssldir=ssl

        BUILD_COMMAND make depend COMMAND make
        INSTALL_COMMAND make install

        INSTALL_DIR ${PREFIX_ext_openssl}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_fftw3} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    )

endif (WIN32)
