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
            URL https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-win64.zip
            URL_HASH SHA256=f5c93c146f52b4f1664fa3ce6579f961a910e869ab0dae431bd871bdd2584ef2

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
            URL https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win32/nasm-2.15.05-win32.zip
            URL_HASH SHA256=258c7d1076e435511cf2fdf94e2281eadbdb9e3003fd57f356f446e2bce3119e

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying nasm 32-bit dependencies
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
    URL https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/nasm-2.15.05.tar.gz
    URL_HASH SHA256=9182a118244b058651c576baa9d0366ee05983c4d4ae1d9ddd3236a9f2304997

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

if(MESON_BINARY_PATH AND NINJA_BINARY_PATH AND RUSTC_BINARY_PATH)
    message(STATUS "Enabling AVIF support.")

    ExternalProject_Add(
        ext_dav1d
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://downloads.videolan.org/pub/videolan/dav1d/0.8.2/dav1d-0.8.2.tar.xz
        URL_HASH SHA256=3dd91d96b44e9d8ba7e82ad9e730d6c579ab5e19edca0db857a60f5ae6a0eb13

        CONFIGURE_COMMAND meson <SOURCE_DIR> --prefix=${EXTPREFIX_heif}

        BUILD_COMMAND meson compile -j${SUBMAKE_JOBS}

        INSTALL_COMMAND meson install

        DEPENDS ext_nasm
    )

    ExternalProject_Add(
        ext_rav1e
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/xiph/rav1e/archive/v0.5.0-alpha.tar.gz
        URL_HASH SHA256=fd884fa387d01860eaf22a0a97353525221eb32bf6b9154bbfa21b6cce5988c8

        CONFIGURE_COMMAND ""

        BUILD_COMMAND cargo cbuild --release --prefix=${EXTPREFIX_heif} --library-type=cdylib -j${SUBMAKE_JOBS}

        INSTALL_COMMAND cargo cinstall --release --library-type=cdylib --prefix=${EXTPREFIX_heif}

        BUILD_IN_SOURCE 1

        DEPENDS ext_nasm
    )

    # External projects do not accept single commands.
    ExternalProject_Add_Step(
        ext_rav1e ext_cargo_c

        COMMENT "Installing cargo-c 0.7.2 to ${EXTPREFIX_heif}"

        COMMAND cargo install cargo-c --version 0.7.2 --root=${EXTPREFIX_heif} -j${SUBMAKE_JOBS}

        DEPENDEES configure
        DEPENDERS build
    )

    set(EXTRA_AVIF_DEPS ext_rav1e ext_dav1d)
else()
    if (NOT MESON_BINARY_PATH)
        message(WARNING "Meson Build system was not found, see https://mesonbuild.com")
    endif()
    if (NOT NINJA_BINARY_PATH)
        message(WARNING "Ninja Build system was not found, see https://ninja-build.org/")
    endif()
    if (NOT RUSTC_BINARY_PATH)
        message(WARNING "Rust compiler was not found, see https://www.rust-lang.org/")
    endif()
    message(WARNING "Disabling AVIF support.")
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

    GIT_REPOSITORY https://github.com/strukturag/libheif.git
    GIT_TAG f5a3d8e090674be349f5c1e24083b3e553c516bd
    GIT_SHALLOW TRUE
    GIT_REMOTE_UPDATE_STRATEGY CHECKOUT

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_EXAMPLES=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${EXTRA_AVIF_DEPS} ext_libde265 ext_libx265 ext_jpeg ext_png
)
