#
# The latest opencolorio doesn't build on Windows without using boost::ptr, but if you build
# it with boost::ptr, you cannot link to it because of missing dll exports, so build an older
# ocio on Windows.
#
SET(EXTPREFIX_ocio "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add(
    ext_ocio
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/OpenColorIO-1.1.0.zip
    URL_MD5 aa52d77b6a4e03017beefaefa613b49f

    INSTALL_DIR ${EXTPREFIX_ocio}
    #PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/patch.diff

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_ocio} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DOCIO_BUILD_APPS=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_DOCS=OFF -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF -DOCIO_BUILD_STATIC_JNIGLUE=OFF

    UPDATE_COMMAND ""
    DEPENDS ext_boost
)
else()
ExternalProject_Add(
    ext_ocio
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/OpenColorIO-1.1.0.tar.gz
    URL_MD5 802d8f5b1d1fe316ec5f76511aa611b8

    INSTALL_DIR ${EXTPREFIX_ocio}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_ocio} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DOCIO_BUILD_APPS=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_DOCS=OFF -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF -DOCIO_BUILD_STATIC_JNIGLUE=OFF

    UPDATE_COMMAND ""
    DEPENDS ext_boost
)
endif()
