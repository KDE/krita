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
    URL http://files.kde.org/krita/build/dependencies/ext_ocio.zip
    URL_MD5 e0bc2c7177dc02bdc82e8f38f52b1058

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
    URL http://files.kde.org/krita/build/dependencies/opencolorio-a557a85454ee1ffa8cb66f8a96238e079c452f08.tgz
    URL_MD5 c7db4cdcac40ab12870cdeb6bbd0b875

    INSTALL_DIR ${EXTPREFIX_ocio}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_ocio} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DOCIO_BUILD_APPS=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_DOCS=OFF -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF -DOCIO_BUILD_STATIC_JNIGLUE=OFF

    UPDATE_COMMAND ""
    ALWAYS 0
    DEPENDS ext_boost
)
endif()
