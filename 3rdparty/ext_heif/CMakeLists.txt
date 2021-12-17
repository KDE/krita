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
        ext_cargo_c
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/lu-zero/cargo-c/archive/refs/tags/v0.8.1.tar.gz
        URL_HASH SHA256=c8eeac1ac6852e551597f38411ac741aeacd6927cc8057e73ed73c7c1e63ee61

        CONFIGURE_COMMAND ""

        BUILD_COMMAND ""

        INSTALL_COMMAND cargo install --path <SOURCE_DIR> -j ${SUBMAKE_JOBS} --root ${EXTPREFIX_heif}

        UPDATE_COMMAND ""

        BUILD_IN_SOURCE 1
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

        DEPENDS ext_nasm ext_cargo_c
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

    find_package(Perl)
    if(NOT PERL_FOUND AND WIN32)
        ExternalProject_Add(
            ext_strawberryperl
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
            # From https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-32bit-portable.zip
            URL https://files.kde.org/krita/build/dependencies/strawberry-perl-5.32.1.1-32bit-portable.zip
            URL_HASH SHA1=28bca91cadd6651c2b2463db8587c170bf17f2fa

            BUILD_COMMAND ${CMAKE_COMMAND} -E echo Deploying Strawberry Perl portable
            CONFIGURE_COMMAND ""
            INSTALL_COMMAND ""
            UPDATE_COMMAND ""

            SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/Strawberry"
        )
        set(USE_STRAWBERRYPERL 1)
        set(LIBAOM_EXTRA_CMAKE_ARGS -DPERL_EXECUTABLE=${CMAKE_CURRENT_BINARY_DIR}/Strawberry/perl/bin/perl.exe)
        set(LIBAOM_STRAWBERRYPERL_DEP ext_strawberryperl)
    endif(NOT PERL_FOUND AND WIN32)

    if(PERL_FOUND OR USE_STRAWBERRYPERL)
        message(WARNING "Building alternative AVIF support using libaom.")

        ExternalProject_Add(
            ext_libaom
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

            URL https://storage.googleapis.com/aom-releases/libaom-3.2.0.tar.gz
            URL_HASH SHA256=8fa7418bf4ea12ef6dcf0eeb21cba932b71c6079ffdf7b9d13cf907fa46c0568

            CMAKE_ARGS ${LIBAOM_EXTRA_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DENABLE_EXAMPLES=OFF -DENABLE_TESTDATA=OFF -DENABLE_TESTS=OFF -DENABLE_TOOLS=OFF ${GLOBAL_PROFILE}

            UPDATE_COMMAND ""

            DEPENDS ext_nasm ${LIBAOM_STRAWBERRYPERL_DEP}
        )

        set(EXTRA_AVIF_DEPS ext_libaom)
    else(PERL_FOUND OR USE_STRAWBERRYPERL)
        message(WARNING "Perl not available, disabling AVIF support.")
    endif(PERL_FOUND OR USE_STRAWBERRYPERL)
endif()

# Incorporate Handbrake's extensions for building x265
# Multicoreware added detection of SSE2/3/4.1, so we can freely enable them now
# Also ship their patch for Apple silicon
if (MSVC)
    set(x265_LIBDIR "$<CONFIG>/")
    set(x265_LIBRARY x265-static.lib)
    set(x265_10bit_LIBRARY x265_main10-static.lib)
    set(x265_12bit_LIBRARY x265_main12-static.lib)
    set(x265_EXTRA_LINK_FLAGS /LIBPATH:${EXTPREFIX_heif}/lib)
else()
    set(x265_LIBDIR "")
    set(x265_LIBRARY libx265.a)
    set(x265_10bit_LIBRARY libx265_main10.a)
    set(x265_12bit_LIBRARY libx265_main12.a)
    set(x265_EXTRA_LINK_FLAGS -L${EXTPREFIX_heif}/lib)
endif()
if ((CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*|arm64.*|ARM64.*)" 
    OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
    AND NOT APPLE)
    # x265 in 32-bit platforms cannot support 10-bit and 12-bit images
    # because they either crash on malloc or require SSE4.1+ extensions
    # Support for 10-bit and 12-bit also requires NEON extensions
    # and these are broken since 3.3
    ExternalProject_Add(
        ext_libx265
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/videolan/x265/archive/3.4.tar.gz
        URL_HASH SHA512=17639324c9428087cda9cfa5b57bcb82403226ec5b4fc0da46028e0700452f7bb12df0f4f3a8fd5d70ebdd912ba7589bd99b01c9b7e0d4fa00424e1502580090

        SOURCE_SUBDIR source/

        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_SHARED=TRUE

        UPDATE_COMMAND ""

        DEPENDS ext_nasm
    )
else()
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
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${x265_LIBDIR}${x265_LIBRARY} ${EXTPREFIX_heif}/lib/${x265_10bit_LIBRARY}
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
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${x265_LIBDIR}${x265_LIBRARY} ${EXTPREFIX_heif}/lib/${x265_12bit_LIBRARY}
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

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_SHARED=TRUE  -DENABLE_HDR10_PLUS=TRUE -DEXTRA_LIB:String=${x265_10bit_LIBRARY}$<SEMICOLON>${x265_12bit_LIBRARY} -DEXTRA_LINK_FLAGS:String=${x265_EXTRA_LINK_FLAGS} -DLINKED_10BIT=TRUE -DLINKED_12BIT=TRUE

    UPDATE_COMMAND ""

    DEPENDS ext_nasm ext_libx265_10bit ext_libx265_12bit
)
endif()

ExternalProject_Add(
    ext_libheif
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    URL https://github.com/strukturag/libheif/archive/8f8035a102e959a3488f69e8befe09d51cf596a7.tar.gz
    URL_HASH SHA256=12cfab1aaa588191bb28a67b275d0e99b379ab8221dcc7a18adcb6a34b661943

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_EXAMPLES=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${EXTRA_AVIF_DEPS} ext_libde265 ext_libx265 ext_jpeg ext_png
)
