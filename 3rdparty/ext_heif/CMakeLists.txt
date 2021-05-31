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
        URL https://downloads.videolan.org/pub/videolan/dav1d/0.9.0/dav1d-0.9.0.tar.xz
        URL_HASH SHA256=cfae88e8067c9b2e5b96d95a7a00155c353376fe9b992a96b4336e0eab19f9f6

        CONFIGURE_COMMAND meson <SOURCE_DIR> --prefix=${EXTPREFIX_heif}

        BUILD_COMMAND meson compile -j${SUBMAKE_JOBS}

        INSTALL_COMMAND meson install

        DEPENDS ext_nasm
    )

    ExternalProject_Add(
        ext_rav1e
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/xiph/rav1e/archive/refs/tags/p20210525.tar.gz
        URL_HASH SHA256=bd1a27e430781d7f1714ea6e73d489b1718d2883936654f665cc57edf1f6e58e

        CONFIGURE_COMMAND ""

        BUILD_COMMAND cargo cbuild --release --prefix=${EXTPREFIX_heif} --library-type=cdylib -j${SUBMAKE_JOBS}

        INSTALL_COMMAND cargo cinstall --release --library-type=cdylib --prefix=${EXTPREFIX_heif}

        BUILD_IN_SOURCE 1

        DEPENDS ext_nasm
    )

    # External projects do not accept single commands.
    ExternalProject_Add_Step(
        ext_rav1e ext_cargo_c

        COMMENT "Installing cargo-c 0.8.1 to ${EXTPREFIX_heif}"

        COMMAND ${CMAKE_CURRENT_LIST_DIR}/cargo-install-wrapper cargo-c --version "0.8.1+cargo-0.53" --root=${EXTPREFIX_heif} -j${SUBMAKE_JOBS}

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

# Incorporate Handbrake's extensions for building x265
# Multicoreware added detection of SSE2/3/4.1, so we can freely enable them now
# Also ship their patch for Apple silicon
ExternalProject_Add(ext_libx265_10bit
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/HandBrake/HandBrake-contribs/releases/download/contribs/x265-snapshot-20210203-12666.tar.gz
    URL_HASH SHA256=a4a712c9fbe724c263f4506e19aeca74e0259b33126d2e60c813352483f5110c

    SOURCE_SUBDIR source/

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A00-darwin-Revert-Add-aarch64-support-Part-2.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A01-darwin-neon-support-for-arm64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A02-threads-priority.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Silence-nasm-2.15-warnings.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-rc-versioning.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Skip-PDB-in-MinGW.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DHIGH_BIT_DEPTH=TRUE -DEXPORT_C_API=FALSE -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy libx265.a ${EXTPREFIX_heif}/lib/libx265_main10.a
)
ExternalProject_Add(ext_libx265_12bit
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/HandBrake/HandBrake-contribs/releases/download/contribs/x265-snapshot-20210203-12666.tar.gz
    URL_HASH SHA256=a4a712c9fbe724c263f4506e19aeca74e0259b33126d2e60c813352483f5110c

    SOURCE_SUBDIR source/

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A00-darwin-Revert-Add-aarch64-support-Part-2.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A01-darwin-neon-support-for-arm64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A02-threads-priority.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Silence-nasm-2.15-warnings.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-rc-versioning.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Skip-PDB-in-MinGW.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} 
    -DHIGH_BIT_DEPTH=TRUE -DMAIN12=TRUE -DEXPORT_C_API=FALSE  -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy libx265.a ${EXTPREFIX_heif}/lib/libx265_main12.a
)
ExternalProject_Add(
    ext_libx265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/HandBrake/HandBrake-contribs/releases/download/contribs/x265-snapshot-20210203-12666.tar.gz
    URL_HASH SHA256=a4a712c9fbe724c263f4506e19aeca74e0259b33126d2e60c813352483f5110c

    SOURCE_SUBDIR source/

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A00-darwin-Revert-Add-aarch64-support-Part-2.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A01-darwin-neon-support-for-arm64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A02-threads-priority.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Silence-nasm-2.15-warnings.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-Fix-rc-versioning.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Skip-PDB-in-MinGW.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_SHARED=TRUE  -DENABLE_HDR10_PLUS=TRUE -DEXTRA_LIB:String=x265_main10.a$<SEMICOLON>x265_main12.a -DEXTRA_LINK_FLAGS:String=-L${EXTPREFIX_heif}/lib -DLINKED_10BIT=TRUE -DLINKED_12BIT=TRUE

    UPDATE_COMMAND ""

    DEPENDS ext_nasm ext_libx265_10bit ext_libx265_12bit
)

ExternalProject_Add(
    ext_libheif
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    URL https://github.com/strukturag/libheif/archive/8f8035a102e959a3488f69e8befe09d51cf596a7.tar.gz
    URL_HASH SHA256=12cfab1aaa588191bb28a67b275d0e99b379ab8221dcc7a18adcb6a34b661943

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_EXAMPLES=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${EXTRA_AVIF_DEPS} ext_libde265 ext_libx265 ext_jpeg ext_png
)
