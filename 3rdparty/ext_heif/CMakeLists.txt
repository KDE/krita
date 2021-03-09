SET(EXTPREFIX_heif "${EXTPREFIX}" )

ExternalProject_Add(
    ext_libde265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libde265/releases/download/v1.0.8/libde265-1.0.8.tar.gz
    URL_HASH SHA1=db8fa137a4681ec092e7546d9155bbaa95938e5e

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Quick-check-for-SSE-support.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)

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


set(EXTRA_AVIF_DEPS "")

find_program(MESON_BINARY_PATH meson)
find_program(NINJA_BINARY_PATH ninja)
find_program(RUSTC_BINARY_PATH rustc)

if (EXISTS ${MESON_BINARY_PATH} AND EXISTS ${MESON_BINARY_PATH} AND EXISTS ${MESON_BINARY_PATH})
    # Assumes an existing Meson/Ninja install
    ExternalProject_Add(
        ext_dav1d
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://code.videolan.org/videolan/dav1d/-/archive/0.8.2/dav1d-0.8.2.tar.gz
        URL_HASH SHA256=00438cf4abb45a56f3dff4c0ff76f4714540672118fe84eea24617d5ed2e394c

        CONFIGURE_COMMAND meson <SOURCE_DIR> --default-library=static --prefix=${EXTPREFIX_heif}

        BUILD_COMMAND meson compile

        INSTALL_COMMAND meson install

        DEPENDS ext_nasm
        )


    # Assumes an existing Rust install
    # cargo install as part of an external configure...?
    ExternalProject_Add(
        ext_rav1e
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/xiph/rav1e/archive/v0.5.0-alpha.tar.gz
        URL_HASH SHA256=fd884fa387d01860eaf22a0a97353525221eb32bf6b9154bbfa21b6cce5988c8

        CONFIGURE_COMMAND cargo install cargo-c

        BUILD_COMMAND ""

        INSTALL_COMMAND cargo cinstall --release --prefix=${EXTPREFIX_heif}

        BUILD_IN_SOURCE 1

        DEPENDS ext_nasm
        )

    set(EXTRA_AVIF_DEPS ext_rav1e ext_dav1d)
else()
    if (NOT EXISTS ${MESON_BINARY_PATH})
        message(WARNING "Meson Build system was not found, see https://mesonbuild.com")
    endif()
    if (NOT EXISTS ${NINJA_BINARY_PATH})
        message(WARNING "Ninja Build system was not found, see https://ninja-build.org/")
    endif()
    if (NOT EXISTS ${RUSTC_BINARY_PATH})
        message(WARNING "Rust compiler was not found, see https://www.rust-lang.org/")
    endif()
endif()

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
    #URL https://github.com/strukturag/libheif/releases/download/v1.11.0/libheif-1.11.0.tar.gz
    #URL_HASH SHA1=def98b8c83861b6e4f3cdf6d3b57c9aaf3408923

    GIT_REPOSITORY https://github.com/strukturag/libheif.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_REMOTE_UPDATE_STRATEGY REBASE

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_EXAMPLES=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${EXTRA_AVIF_DEPS} ext_libde265 ext_libx265 ext_jpeg ext_png
)
