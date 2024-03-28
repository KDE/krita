SET(EXTPREFIX_openexr "${EXTPREFIX}" )
if (WIN32)
    set(_openexr_dep ${ZLIB_DEP})
else()
    set(_openexr_dep)
endif()

ExternalProject_Add( ext_openexr
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    DOWNLOAD_NAME openexr-v2.5.8.tar.gz
    URL https://github.com/AcademySoftwareFoundation/openexr/archive/v2.5.8.tar.gz
    URL_HASH SHA256=db261a7fcc046ec6634e4c5696a2fc2ce8b55f50aac6abe034308f54c8495f55

    INSTALL_DIR ${EXTPREFIX_openexr}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

    UPDATE_COMMAND ""
    DEPENDS ${_openexr_dep}
)
