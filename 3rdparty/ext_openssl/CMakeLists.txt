SET(PREFIX_ext_openssl "${EXTPREFIX}" )

if (WIN32)
    # Use the binaries from curl for Windows (https://github.com/curl/curl-for-win).
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(ext_openssl
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://curl.se/windows/dl-7.75.0_2/openssl-1.1.1i_2-win64-mingw.zip
            URL_HASH SHA256=888b96e922278e49fe626c75b963e3b92ea991a8eae1c0aa6bba11a18e8bba1d

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
            URL https://curl.se/windows/dl-7.75.0_2/openssl-1.1.1i_2-win32-mingw.zip
            URL_HASH SHA256=422846b60665dbb02bcf5d93ec0f36ff5de8db62c02cb23834e5973168409cb1

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
        URL https://www.openssl.org/source/openssl-1.1.1i.tar.gz
        URL_HASH SHA1=eb684ba4ed31fe2c48062aead75233ecd36882a6

        CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${PREFIX_ext_openssl} --openssldir=ssl

        BUILD_COMMAND make depend COMMAND make
        INSTALL_COMMAND make install

        INSTALL_DIR ${PREFIX_ext_openssl}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_fftw3} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

        UPDATE_COMMAND ""
    )

endif (WIN32)
