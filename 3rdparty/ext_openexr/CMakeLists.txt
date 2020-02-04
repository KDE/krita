SET(EXTPREFIX_openexr "${EXTPREFIX}" )
if (MINGW)
ExternalProject_Add( ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/AcademySoftwareFoundation/openexr/archive/v2.4.0.tar.gz
        URL_MD5 9e4d69cf2a12c6fb19b98af7c5e0eaee

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/openexr.diff
              COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/symlink.patch
              COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/exr_symlink.diff

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DILMBASE_PACKAGE_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        DEPENDS ext_ilmbase ext_zlib
)
else()
ExternalProject_Add(ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/AcademySoftwareFoundation/openexr/archive/v2.4.0.tar.gz
        URL_MD5 9e4d69cf2a12c6fb19b98af7c5e0eaee

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/openexr.diff

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DILMBASE_PACKAGE_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        DEPENDS ext_ilmbase
)
endif()
