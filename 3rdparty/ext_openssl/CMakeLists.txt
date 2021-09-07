SET(PREFIX_ext_openssl "${EXTPREFIX}" )

if (WIN32)
    
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        if (MSVC)
        # Use the binaries from ConanCenter
            ExternalProject_Add(ext_openssl
                DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                URL https://files.kde.org/krita/build/dependencies/openssl-1.1.1k-win64-msvc.tgz
                URL_HASH SHA256=beae4c78e2f2b64cc3621402a34550ef1fffd08cd35b6a6802f22ed0460ff1d4

                BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying openssl 64-bit dependencies
                CONFIGURE_COMMAND ""
                INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${PREFIX_ext_openssl}/include
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libcrypto.lib ${PREFIX_ext_openssl}/lib
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libssl.lib ${PREFIX_ext_openssl}/lib
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/libcrypto-1_1-x64.dll ${PREFIX_ext_openssl}/bin
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin/libssl-1_1-x64.dll ${PREFIX_ext_openssl}/bin
                UPDATE_COMMAND ""
            )
        else(MSVC)
        # Use the binaries from curl for Windows (https://github.com/curl/curl-for-win).
            ExternalProject_Add(ext_openssl
                DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
                URL https://files.kde.org/krita/build/dependencies/openssl-1.1.1l_4-win64-mingw.zip
                URL_HASH SHA256=6582432d4f537323bfa032a1d91b4eee12efdf0c72e4fabdacc0ac5dd1685e9a

                BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying openssl 64-bit dependencies
                CONFIGURE_COMMAND ""
                INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${PREFIX_ext_openssl}/include
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libcrypto.dll.a ${PREFIX_ext_openssl}/lib
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/lib/libssl.dll.a ${PREFIX_ext_openssl}/lib
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/libcrypto-1_1-x64.dll ${PREFIX_ext_openssl}/bin
                        COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/libssl-1_1-x64.dll ${PREFIX_ext_openssl}/bin
                UPDATE_COMMAND ""
            )
        endif(MSVC)
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
elseif (ANDROID)
    ExternalProject_Add( ext_openssl
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://www.openssl.org/source/openssl-1.1.1i.tar.gz
        URL_HASH SHA1=eb684ba4ed31fe2c48062aead75233ecd36882a6

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
            ANDROID_NDK_HOME=${CMAKE_ANDROID_NDK}
            PATH=${ANDROID_TOOLCHAIN_ROOT}/bin:$ENV{PATH}
            CXXFLAGS=-D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
            <SOURCE_DIR>/Configure android-${CMAKE_ANDROID_ARCH} --prefix=${PREFIX_ext_openssl} --openssldir=ssl

        BUILD_COMMAND ${CMAKE_COMMAND} -E env
            PATH=${ANDROID_TOOLCHAIN_ROOT}/bin:$ENV{PATH}
            CXXFLAGS=-D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
            make build_libs SHLIB_VERSION_NUMBER= SHLIB_EXT=_1_1.so
        INSTALL_COMMAND ${CMAKE_COMMAND} -E env
            PATH=${ANDROID_TOOLCHAIN_ROOT}/bin:$ENV{PATH}
            CXXFLAGS=-D__ANDROID_API__=${ANDROID_PLATFORM_LEVEL}
            make install_sw SHLIB_VERSION_NUMBER= SHLIB_EXT=_1_1.so

        INSTALL_DIR ${PREFIX_ext_openssl}

        UPDATE_COMMAND ""
)
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
