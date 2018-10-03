SET(PREFIX_ext_lzma "${EXTPREFIX}" )
if (MINGW)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8") # 64 bits
        ExternalProject_Add( ext_lzma
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://tukaani.org/xz/xz-5.2.4-windows.zip
            URL_MD5 3a4467e74e1eafb89b517aae7044e46c

            INSTALL_DIR ${PREFIX_ext_lzma}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying 64-bit liblzma.dll
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_x86-64/liblzma.dll ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_x86-64/liblzma.a ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/filter.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/hardware.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/index_hash.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/version.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/delta.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/index.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/check.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/stream_flags.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/bcj.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/lzma12.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/block.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/base.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/container.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/vli.h ${PREFIX_ext_lzma}/include
            
            UPDATE_COMMAND ""
        )
    
    else() # 32 bits

            ExternalProject_Add( ext_lzma
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://tukaani.org/xz/xz-5.2.4-windows.zip
            URL_MD5 3a4467e74e1eafb89b517aae7044e46c
            
            INSTALL_DIR ${PREFIX_ext_lzma}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying 32-bit liblzma.dll
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_i686-sse2/liblzma.dll ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/bin_i686-sse2/liblzma.a ${PREFIX_ext_lzma}/bin/
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/filter.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/hardware.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/index_hash.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/version.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/delta.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/index.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/check.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/stream_flags.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/bcj.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/lzma12.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/block.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/base.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/container.h ${PREFIX_ext_lzma}/include
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/include/lzma/vli.h ${PREFIX_ext_lzma}/include


            UPDATE_COMMAND ""
        )
    
    endif()
else ()
    ExternalProject_Add( ext_lzma
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://tukaani.org/xz/xz-5.2.3.tar.xz
        URL_MD5 60fb79cab777e3f71ca43d298adacbd5
        
        INSTALL_DIR ${PREFIX_ext_lzma}
        CONFIGURE_COMMAND ./configure --prefix=${PREFIX_ext_lzma} --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts --disable-doc --disable-static --enable-shared ${GLOBAL_AUTOMAKE_PROFILE}  
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE 1
    )

endif ()
