SET(PREFIX_ext_freetype "${EXTPREFIX}" )
if(APPLE OR UNIX)
ExternalProject_Add( ext_freetype
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.10.4.tar.xz
    URL_HASH SHA256=86a854d8905b19698bbc8f23b860bc104246ce4854dcea8e3b0fb21284f75784

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_freetype} ${GLOBAL_AUTOMAKE_PROFILE}
    BUILD_COMMAND make
    INSTALL_COMMAND make install

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_freetype
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.10.4.tar.xz
    URL_HASH SHA256=86a854d8905b19698bbc8f23b860bc104246ce4854dcea8e3b0fb21284f75784

    INSTALL_DIR ${PREFIX_ext_freetype}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_freetype} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
endif()
