SET(EXTPREFIX_heif "${EXTPREFIX}" )
ExternalProject_Add(
    ext_libde265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libde265/releases/download/v1.0.3/libde265-1.0.3.tar.gz
    URL_MD5 0e1e26ffcb2177c3749c748e20fcd588

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXTPREFIX_heif} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)

ExternalProject_Add(
    ext_yasm
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
    URL_MD5 fc9e586751ff789b34b1f21d572d96af

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXTPREFIX_heif} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)

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

ExternalProject_Add(
    ext_libx265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://ftp.videolan.org/pub/videolan/x265/x265_3.0.tar.gz
    URL_MD5 8ff1780246bb7ac8506239f6129c04ec

    SOURCE_SUBDIR source/
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_heif}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
    DEPENDS ext_yasm ext_nasm
)

ExternalProject_Add(
    ext_libheif
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libheif/releases/download/v1.4.0/libheif-1.4.0.tar.gz
    URL_MD5 b3c726609a8750c5fc1687a7b193adff

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${EXTPREFIX_heif} ${GLOBAL_AUTOMAKE_PROFILE}

    BUILD_COMMAND CXXFLAGS="-Wno-error" make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
    DEPENDS ext_libde265 ext_libx265 ext_pkgconfig
)
