SET(PREFIX_ext_lzma "${EXTPREFIX}" )
if (WIN32)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8") # 64 bits
        ExternalProject_Add( ext_lzma
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://tukaani.org/xz/xz-5.2.5-windows.zip
            URL_HASH SHA256=d83b82ca75dfab39a13dda364367b34970c781a9df4d41264db922ac3a8f622d

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
            URL https://tukaani.org/xz/xz-5.2.5-windows.zip
            URL_HASH SHA256=d83b82ca75dfab39a13dda364367b34970c781a9df4d41264db922ac3a8f622d

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
        URL https://tukaani.org/xz/xz-5.2.5.tar.gz
        URL_HASH SHA256=f6f4910fd033078738bd82bfba4f49219d03b17eb0794eb91efbae419f4aba10

        INSTALL_DIR ${PREFIX_ext_lzma}
        CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_lzma} --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts --disable-doc --disable-static --enable-shared ${GLOBAL_AUTOMAKE_PROFILE}
        BUILD_COMMAND make
        INSTALL_COMMAND make install

        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
    )

endif ()
