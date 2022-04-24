SET(PREFIX_ext_freetype "${EXTPREFIX}" )
if(APPLE OR UNIX)
ExternalProject_Add( ext_freetype
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.11.1.tar.xz
    URL_HASH SHA256=3333ae7cfda88429c97a7ae63b7d01ab398076c3b67182e960e5684050f2c5c8

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_freetype} ${GLOBAL_AUTOMAKE_PROFILE}

    UPDATE_COMMAND ""
)
else()
ExternalProject_Add( ext_freetype
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.11.1.tar.xz
    URL_HASH SHA256=3333ae7cfda88429c97a7ae63b7d01ab398076c3b67182e960e5684050f2c5c8
    
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Publish-FreeType-dependencies.patch

    INSTALL_DIR ${PREFIX_ext_freetype}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_freetype} -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=ON -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DFT_WITH_PNG=TRUE ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
endif()
