SET(EXTPREFIX_heif "${EXTPREFIX}" )
if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

if(APPLE AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
    set(SSE_OPTIMIZATIONS_STRING "-DDISABLE_SSE=TRUE" "-DSUPPORTS_SSE4_1=OFF")
else()
    set(SSE_OPTIMIZATIONS_STRING "")
endif()

ExternalProject_Add(
    ext_libde265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libde265/releases/download/v1.0.5/libde265-1.0.5.tar.gz
    URL_MD5 385c88166cb255a068a8c936d0ab23ef

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01-fix-sse41.diff

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} ${SSE_OPTIMIZATIONS_STRING}

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
        URL http://ftp.videolan.org/pub/videolan/x265/x265_3.2.1.tar.gz
        URL_MD5 94808045a34d88a857e5eaf3f68f4bca

        SOURCE_SUBDIR source/

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DHIGH_BIT_DEPTH=TRUE -DEXPORT_C_API=FALSE -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy libx265.a ${EXTPREFIX_heif}/lib/libx265_main10.a
    )
    ExternalProject_Add(ext_libx265_12bit
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://ftp.videolan.org/pub/videolan/x265/x265_3.2.1.tar.gz
        URL_MD5 94808045a34d88a857e5eaf3f68f4bca

        SOURCE_SUBDIR source/

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} 
        -DHIGH_BIT_DEPTH=TRUE -DMAIN12=TRUE -DEXPORT_C_API=FALSE  -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy libx265.a ${EXTPREFIX_heif}/lib/libx265_main12.a
    )
    ExternalProject_Add(
        ext_libx265
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://ftp.videolan.org/pub/videolan/x265/x265_3.2.1.tar.gz
        URL_MD5 94808045a34d88a857e5eaf3f68f4bca

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
        URL http://ftp.videolan.org/pub/videolan/x265/x265_3.2.1.tar.gz
        URL_MD5 94808045a34d88a857e5eaf3f68f4bca

        SOURCE_SUBDIR source/

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/02-skip-pdb-in-mingw.diff
            COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-arm-asm-primitives.patch

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_SHARED=TRUE -DENABLE_CLI=FALSE

        UPDATE_COMMAND ""

        DEPENDS ext_nasm
    )
endif ()

ExternalProject_Add(
    ext_libheif
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libheif/releases/download/v1.6.2/libheif-1.6.2.tar.gz
    URL_HASH SHA256=bb229e855621deb374f61bee95c4642f60c2a2496bded35df3d3c42cc6d8eefc

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/03-fix-build-macos-windows.diff

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_EXAMPLES=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ext_libde265 ext_libx265 ext_jpeg ext_png
)
