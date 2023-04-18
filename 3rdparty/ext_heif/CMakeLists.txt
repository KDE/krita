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
else (WIN32)
ExternalProject_Add(
    ext_nasm
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/nasm-2.15.05.tar.gz
    URL_HASH SHA256=9182a118244b058651c576baa9d0366ee05983c4d4ae1d9ddd3236a9f2304997

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXTPREFIX_heif} ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
    
    # automake has an issue when tryin gto build outside of the course tree
    # 'sh: 0: Can't open autogen.sh'
    BUILD_IN_SOURCE ON
)
endif(WIN32)


set(EXTRA_AVIF_DEPS "")

if (WIN32 AND NOT PERL_FOUND) # Implies strawberry perl
    set(LIBAOM_STRAWBERRYPERL_DEP ext_strawberryperl)
endif()

if(WIN32 AND PERL_EXECUTABLE)
    set(LIBAOM_EXTRA_CMAKE_ARGS -DPERL_EXECUTABLE=${PERL_EXECUTABLE})
elseif (NOT WIN32)
    set(LIBAOM_EXTRA_CMAKE_ARGS -DCONFIG_PIC=1)
endif()

if(PERL_FOUND OR USE_STRAWBERRYPERL)
    message(STATUS "Building AVIF support using libaom.")

    if(APPLE)
        kis_ExternalProject_Add_macos(
            ext_libaom
            URL https://storage.googleapis.com/aom-releases/libaom-3.6.0.tar.gz
            URL_HASH SHA256=a4a6c0fab685da743b796662a928fcdf7ae60594edc306efb73e78a17ea6cde6

            # Shared libraries cause a race condition in MSVC because
            # both export and static targets output the same file.
            # Do NOT build them that way unless you want the FFmpeg build
            # to fail in turn!
            CONFIGURE_ARGS ${LIBAOM_EXTRA_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif}
                -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF
                -DENABLE_EXAMPLES=OFF -DENABLE_TESTDATA=OFF -DENABLE_TESTS=OFF 
                -DENABLE_TOOLS=OFF ${GLOBAL_PROFILE}

            DEPENDS ext_nasm ${LIBAOM_STRAWBERRYPERL_DEP}
        )

    else()
        ExternalProject_Add(
            ext_libaom
            DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

            URL https://storage.googleapis.com/aom-releases/libaom-3.6.0.tar.gz
            URL_HASH SHA256=a4a6c0fab685da743b796662a928fcdf7ae60594edc306efb73e78a17ea6cde6

            # Shared libraries cause a race condition in MSVC because
            # both export and static targets output the same file.
            # Do NOT build them that way unless you want the FFmpeg build
            # to fail in turn!
            CMAKE_ARGS ${LIBAOM_EXTRA_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF -DENABLE_EXAMPLES=OFF -DENABLE_TESTDATA=OFF -DENABLE_TESTS=OFF -DENABLE_TOOLS=OFF ${GLOBAL_PROFILE}

            UPDATE_COMMAND ""

            DEPENDS ext_nasm ${LIBAOM_STRAWBERRYPERL_DEP}
        )
    endif()

    set(EXTRA_AVIF_DEPS ext_libaom)
else()
    message(WARNING "Perl not available, disabling AVIF support.")
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
ExternalProject_Add(ext_libx265_10bit
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/HandBrake/HandBrake-contribs/releases/download/contribs/x265-snapshot-20220709-12716.tar.gz
    URL_HASH SHA256=19ef8b21301f13d795601184884859e9f4a2f87fcda83757bd7dcd588056e926

    SOURCE_SUBDIR source/

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A01-build-fix.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A02-threads-priority.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A03-threads-pool-adjustments.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A04-64-bit_for_aarch64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A05-disable-scanPosLast-aarch64-asm.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A06-common-threading.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Silence-nasm-2.15-warnings.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Skip-PDB-in-MinGW.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DHIGH_BIT_DEPTH=TRUE -DEXPORT_C_API=FALSE -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${x265_LIBDIR}${x265_LIBRARY} ${EXTPREFIX_heif}/lib/${x265_10bit_LIBRARY}
    UPDATE_COMMAND ""

    DEPENDS ext_nasm
)
ExternalProject_Add(ext_libx265_12bit
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/HandBrake/HandBrake-contribs/releases/download/contribs/x265-snapshot-20220709-12716.tar.gz
    URL_HASH SHA256=19ef8b21301f13d795601184884859e9f4a2f87fcda83757bd7dcd588056e926

    SOURCE_SUBDIR source/

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A01-build-fix.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A02-threads-priority.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A03-threads-pool-adjustments.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A04-64-bit_for_aarch64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A05-disable-scanPosLast-aarch64-asm.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A06-common-threading.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Silence-nasm-2.15-warnings.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Skip-PDB-in-MinGW.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} 
    -DHIGH_BIT_DEPTH=TRUE -DMAIN12=TRUE -DEXPORT_C_API=FALSE  -DENABLE_CLI=FALSE -DENABLE_SHARED=FALSE
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${x265_LIBDIR}${x265_LIBRARY} ${EXTPREFIX_heif}/lib/${x265_12bit_LIBRARY}
    UPDATE_COMMAND ""

    DEPENDS ext_nasm ext_libx265_10bit
)
ExternalProject_Add(
    ext_libx265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/HandBrake/HandBrake-contribs/releases/download/contribs/x265-snapshot-20220709-12716.tar.gz
    URL_HASH SHA256=19ef8b21301f13d795601184884859e9f4a2f87fcda83757bd7dcd588056e926

    SOURCE_SUBDIR source/

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A01-build-fix.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A02-threads-priority.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A03-threads-pool-adjustments.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A04-64-bit_for_aarch64.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A05-disable-scanPosLast-aarch64-asm.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/A06-common-threading.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Silence-nasm-2.15-warnings.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-Skip-PDB-in-MinGW.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_SHARED=TRUE  -DENABLE_HDR10_PLUS=TRUE -DEXTRA_LIB:String=${x265_10bit_LIBRARY}$<SEMICOLON>${x265_12bit_LIBRARY} -DEXTRA_LINK_FLAGS:String=${x265_EXTRA_LINK_FLAGS} -DLINKED_10BIT=TRUE -DLINKED_12BIT=TRUE

    UPDATE_COMMAND ""

    DEPENDS ext_nasm ext_libx265_12bit
)

ExternalProject_Add(
    ext_libheif
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    URL https://github.com/strukturag/libheif/releases/download/v1.13.0/libheif-1.13.0.tar.gz
    URL_HASH SHA256=c20ae01bace39e89298f6352f1ff4a54b415b33b9743902da798e8a1e51d7ca1

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/clang-aom-link-pthread.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DWITH_EXAMPLES=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ${EXTRA_AVIF_DEPS} ext_libde265 ext_libx265
)
