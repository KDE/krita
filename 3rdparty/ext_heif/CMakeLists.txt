SET(EXTPREFIX_heif "${EXTPREFIX}" )
if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
ExternalProject_Add(
    ext_libde265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libde265/releases/download/v1.0.5/libde265-1.0.5.tar.gz
    URL_MD5 385c88166cb255a068a8c936d0ab23ef

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-fix-sse41.diff

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
else ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
ExternalProject_Add(
    ext_libde265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libde265/releases/download/v1.0.5/libde265-1.0.5.tar.gz
    URL_MD5 385c88166cb255a068a8c936d0ab23ef

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-fix-sse41.diff

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DDISABLE_SSE=TRUE

    UPDATE_COMMAND ""
)
endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

if (WIN32)
    if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(
            ext_nasm
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.nasm.us/pub/nasm/releasebuilds/2.14.03rc2/win64/nasm-2.14.03rc2-win64.zip
            URL_MD5 f12d14ca46cf397a1d27ac0edde548fb

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying nasm 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/rdoff ${EXTPREFIX_heif}/bin/rdoff
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/nasm.exe ${EXTPREFIX_heif}/bin/nasm.exe
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/ndisasm.exe ${EXTPREFIX_heif}/bin/ndisasm.exe

            UPDATE_COMMAND ""
        )
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        ExternalProject_Add(
            ext_nasm
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            URL https://www.nasm.us/pub/nasm/releasebuilds/2.14.03rc2/win32/nasm-2.14.03rc2-win32.zip
            URL_MD5 9ec22ac06a6b9d993ef503ea2fe074ab

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying nasm 64-bit dependencies
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/rdoff ${EXTPREFIX_heif}/bin/rdoff
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/nasm.exe ${EXTPREFIX_heif}/bin/nasm.exe
                    COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/ndisasm.exe ${EXTPREFIX_heif}/bin/ndisasm.exe

            UPDATE_COMMAND ""
        )
    endif ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
else (WIN32)
ExternalProject_Add(
    ext_nasm
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.nasm.us/pub/nasm/releasebuilds/2.14.03rc2/nasm-2.14.03rc2.tar.gz
    URL_MD5 4cd1fe6788cd15d08c1a8f18b6d2428e

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXTPREFIX_heif} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)
endif(WIN32)

IF ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*|arm64.*|ARM64.*)")
    ExternalProject_Add(ext_libx265_10bit
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://bitbucket.org/multicoreware/x265/downloads/x265_3.4.tar.gz
        URL_MD5 e37b91c1c114f8815a3f46f039fe79b5

        SOURCE_SUBDIR source/

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DHIGH_BIT_DEPTH=TRUE -DEXPORT_C_API=FALSE -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy libx265.a ${EXTPREFIX_heif}/lib/libx265_main10.a
    )
    ExternalProject_Add(ext_libx265_12bit
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://bitbucket.org/multicoreware/x265/downloads/x265_3.4.tar.gz
        URL_MD5 e37b91c1c114f8815a3f46f039fe79b5

        SOURCE_SUBDIR source/

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} 
        -DHIGH_BIT_DEPTH=TRUE -DMAIN12=TRUE -DEXPORT_C_API=FALSE  -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy libx265.a ${EXTPREFIX_heif}/lib/libx265_main12.a
    )
    ExternalProject_Add(
        ext_libx265
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://bitbucket.org/multicoreware/x265/downloads/x265_3.4.tar.gz
        URL_MD5 e37b91c1c114f8815a3f46f039fe79b5

        SOURCE_SUBDIR source/

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-skip-pdb-in-mingw.diff

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_SHARED=TRUE  -DENABLE_HDR10_PLUS=TRUE -DEXTRA_LIB:String=x265_main10.a$<SEMICOLON>x265_main12.a -DEXTRA_LINK_FLAGS:String=-L${EXTPREFIX_heif}/lib -DLINKED_10BIT=TRUE -DLINKED_12BIT=TRUE

        UPDATE_COMMAND ""

        DEPENDS ext_nasm ext_libx265_10bit ext_libx265_12bit
    )
else ()
    # x265 in 32-bit platforms cannot support 10-bit and 12-bit images
    # because they either crash on malloc or require SSE4.1+ extensions
    # Same for aarch64, support for 10-bit and 12-bit requires NEON extensions
    # and these are broken as of 3.4
    ExternalProject_Add(
        ext_libx265
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://bitbucket.org/multicoreware/x265/downloads/x265_3.4.tar.gz
        URL_MD5 e37b91c1c114f8815a3f46f039fe79b5

        SOURCE_SUBDIR source/

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-skip-pdb-in-mingw.diff

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_SHARED=TRUE -DENABLE_CLI=FALSE

        UPDATE_COMMAND ""

        DEPENDS ext_nasm
    )
endif ()

ExternalProject_Add(
    ext_libheif
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libheif/releases/download/v1.7.0/libheif-1.7.0.tar.gz
    URL_HASH SHA256=842a9ab4b8d6f0faf5a6dc5e8507321199ec44c0b1d8eb199f2de9b49e2db092

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/03-fix-build-macos-windows.diff

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_EXAMPLES=ON ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ext_libde265 ext_libx265 ext_jpeg ext_png
)
