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
    URL https://github.com/AcademySoftwareFoundation/OpenColorIO/archive/v1.1.1.tar.gz
    URL_MD5 23d8b9ac81599305539a5a8674b94a3d

    INSTALL_DIR ${EXTPREFIX_ocio}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/ocio-mingw.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Auto-disable-SSE-if-the-compiler-target-doesn-t-supp.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_ocio} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DOCIO_BUILD_APPS=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_DOCS=OFF -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF -DOCIO_BUILD_STATIC=OFF -DOCIO_BUILD_JNIGLUE=OFF

    UPDATE_COMMAND ""
    DEPENDS ext_boost
)
else()

if(APPLE AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
    set(SSE_OPTIMIZATIONS_STRING "-DCMAKE_OSX_ARCHITECTURES=arm64")
else()
    set(SSE_OPTIMIZATIONS_STRING "")
endif()

ExternalProject_Add(
    ext_ocio
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/AcademySoftwareFoundation/OpenColorIO/archive/v1.1.1.tar.gz
    URL_MD5 23d8b9ac81599305539a5a8674b94a3d

    INSTALL_DIR ${EXTPREFIX_ocio}
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/warnings.diff
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Auto-disable-SSE-if-the-compiler-target-doesn-t-supp.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_ocio} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DOCIO_BUILD_APPS=OFF -DOCIO_BUILD_TRUELIGHT=OFF -DOCIO_BUILD_NUKE=OFF -DOCIO_BUILD_DOCS=OFF -DOCIO_BUILD_TESTS=OFF -DOCIO_BUILD_PYGLUE=OFF -DOCIO_BUILD_STATIC_JNIGLUE=OFF ${SSE_OPTIMIZATIONS_STRING}

    UPDATE_COMMAND ""
    DEPENDS ext_boost
)
endif()
