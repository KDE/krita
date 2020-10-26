SET(EXTPREFIX_openexr "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add( ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/AcademySoftwareFoundation/openexr/archive/v2.5.3.tar.gz
        URL_HASH SHA256=6a6525e6e3907715c6a55887716d7e42d09b54d2457323fcee35a0376960bebf

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        DEPENDS ext_zlib
)
else()
ExternalProject_Add(ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/AcademySoftwareFoundation/openexr/archive/v2.5.3.tar.gz
        URL_HASH SHA256=6a6525e6e3907715c6a55887716d7e42d09b54d2457323fcee35a0376960bebf

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
)
endif()
