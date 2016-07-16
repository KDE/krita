#
# The latest opencolorio doesn't build on Windows without using boost::ptr, but if you build
# it with boost::ptr, you cannot link to it because of missing dll exports, so build an older
# ocio on Windows.
#
SET(EXTPREFIX_ocio "${EXTPREFIX}" )
if (MSVC)
ExternalProject_Add(
    ext_ocio
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/imageworks-OpenColorIO-v1.0.8-0-g19ed2e3.zip
    URL_MD5 084584fb45d2979048e7acc2f10c756b

    INSTALL_DIR ${EXTPREFIX_ocio}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_ocio} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DOCIO_BUILD_APPS=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_DOCS=OFF -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF -DOCIO_BUILD_STATIC_JNIGLUE=OFF 

    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_boost
)
else()
ExternalProject_Add(
    ext_ocio
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/imageworks-OpenColorIO-v1.0.8-0-g19ed2e3.tar.gz
    URL_MD5 9838f5f1cf624a3d1253241f22e2179a

    INSTALL_DIR ${EXTPREFIX_ocio}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_ocio} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DOCIO_BUILD_APPS=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_DOCS=OFF -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF -DOCIO_BUILD_STATIC_JNIGLUE=OFF

    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_boost
)
endif()
