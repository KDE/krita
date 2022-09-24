SET(PREFIX_ext_lzma "${EXTPREFIX}" )
if (WIN32)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8") # 64 bits
        ExternalProject_Add( ext_lzma
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://tukaani.org/xz/xz-5.2.6-windows.zip
            URL_HASH SHA256=6fd6dad79e5b0d3b24a43bc3f79472b62a48d210f1aaa02fb06e5dfad89a4ebc

            INSTALL_DIR ${PREFIX_ext_lzma}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying 64-bit liblzma.dll
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_x86-64/liblzma.dll ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_x86-64/liblzma.a ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma.h ${PREFIX_ext_lzma}/include/
                    COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include/lzma ${PREFIX_ext_lzma}/include/lzma/

            UPDATE_COMMAND ""
        )

    else() # 32 bits

            ExternalProject_Add( ext_lzma
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://tukaani.org/xz/xz-5.2.6-windows.zip
            URL_HASH SHA256=6fd6dad79e5b0d3b24a43bc3f79472b62a48d210f1aaa02fb06e5dfad89a4ebc

            INSTALL_DIR ${PREFIX_ext_lzma}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying 32-bit liblzma.dll
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_i686-sse2/liblzma.dll ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_i686-sse2/liblzma.a ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma.h ${PREFIX_ext_lzma}/include/
                    COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include/lzma ${PREFIX_ext_lzma}/include/lzma/



            UPDATE_COMMAND ""
        )

    endif()
else ()
    ExternalProject_Add( ext_lzma
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://tukaani.org/xz/xz-5.2.6.tar.gz
        URL_HASH SHA256=a2105abee17bcd2ebd15ced31b4f5eda6e17efd6b10f921a01cda4a44c91b3a0

        INSTALL_DIR ${PREFIX_ext_lzma}
        CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_lzma} --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts --disable-doc --disable-static --enable-shared ${GLOBAL_AUTOMAKE_PROFILE}

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
    )

endif ()
