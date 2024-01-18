SET(PREFIX_ext_jpegxl "${EXTPREFIX}" )

if (MINGW)
    include(CheckCXXSymbolExists)
    include(CheckCXXCompilerFlag)
    check_cxx_symbol_exists(PRIu64 "inttypes.h" CAN_BUILD_HIGHWAY)
    check_cxx_compiler_flag("-mavx512f" CAN_TARGET_AVX512F)
    if (NOT CAN_BUILD_HIGHWAY OR NOT CAN_TARGET_AVX512F)
        message(WARNING "Skipping libjxl, compiler cannot build highway.")
        return()
    endif()
endif()

ExternalProject_Add( ext_highway
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/google/highway/archive/refs/tags/1.0.7.tar.gz
    URL_HASH SHA256=5434488108186c170a5e2fca5e3c9b6ef59a1caa4d520b008a9b8be6b8abe6c5

    INSTALL_DIR ${PREFIX_ext_jpegxl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpegxl} DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF ${GLOBAL_PROFILE} 

    UPDATE_COMMAND ""
)

ExternalProject_Add( ext_brotli
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/google/brotli/archive/v1.1.0.tar.gz
    URL_HASH SHA256=e720a6ca29428b803f4ad165371771f5398faba397edf6778837a18599ea13ff

    INSTALL_DIR ${PREFIX_ext_jpegxl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpegxl} DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_TESTING=OFF ${GLOBAL_PROFILE} 

    UPDATE_COMMAND ""
)

ExternalProject_Add( ext_jpegxl
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libjxl/libjxl/archive/refs/tags/v0.9.1.tar.gz
    URL_HASH SHA256=a0e72e9ece26878147069ad4888ac3382021d4bbee71c2e1b687d5bde7fd7e01

    INSTALL_DIR ${PREFIX_ext_jpegxl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_jpegxl} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
        -DJPEGXL_VERSION=0.9.1
        -DJPEGXL_ENABLE_BENCHMARK=OFF
        -DJPEGXL_ENABLE_COVERAGE=OFF
        -DJPEGXL_ENABLE_EXAMPLES=OFF
        -DJPEGXL_ENABLE_FUZZERS=OFF
        -DJPEGXL_ENABLE_JNI=OFF
        -DJPEGXL_ENABLE_SJPEG=OFF
        -DJPEGXL_ENABLE_SKCMS=OFF
        -DJPEGXL_ENABLE_TOOLS=OFF
        -DJPEGXL_ENABLE_VIEWERS=OFF
        -DJPEGXL_ENABLE_JPEGLI_LIBJPEG=OFF
        -DJPEGXL_ENABLE_JPEGLI=OFF
        "-DCMAKE_CXX_FLAGS:STRING=-DJXL_DEBUG_ON_ERROR -DJX_DEBUG_WARNING" ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    DEPENDS ext_brotli ext_highway ext_lcms2
)
