SET(EXTPREFIX_openexr "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add( ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://api.github.com/repos/AcademySoftwareFoundation/openexr/tarball/v2.4.0
        URL_MD5 a22f9f2eeed5ef20e18b68018af0d429

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/openexr.diff
            # COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/patch_mingw.patch

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DILMBASE_PACKAGE_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        DEPENDS ext_ilmbase ext_zlib
)
else()
ExternalProject_Add(ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://api.github.com/repos/AcademySoftwareFoundation/openexr/tarball/v2.4.0
        URL_MD5 a22f9f2eeed5ef20e18b68018af0d429

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/openexr.diff

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DILMBASE_PACKAGE_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        DEPENDS ext_ilmbase
)
endif()
