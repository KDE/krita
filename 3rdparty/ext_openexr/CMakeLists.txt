SET(EXTPREFIX_openexr "${EXTPREFIX}" )
if (WIN32)
ExternalProject_Add( ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/AcademySoftwareFoundation/openexr/archive/v2.5.5.tar.gz
        URL_HASH SHA256=59e98361cb31456a9634378d0f653a2b9554b8900f233450f2396ff495ea76b3

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        DEPENDS ext_zlib
)
else()
ExternalProject_Add(ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/AcademySoftwareFoundation/openexr/archive/v2.5.5.tar.gz
        URL_HASH SHA256=59e98361cb31456a9634378d0f653a2b9554b8900f233450f2396ff495ea76b3

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
)
endif()
