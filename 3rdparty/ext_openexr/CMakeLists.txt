SET(EXTPREFIX_openexr "${EXTPREFIX}" )
if (MSVC OR MINGW)
ExternalProject_Add( ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/openexr-2.2.0.tar.gz
        URL_MD5 b64e931c82aa3790329c21418373db4e

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/openexr.diff
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/patch_mingw.patch

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DILMBASE_PACKAGE_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        ALWAYS 0
        DEPENDS ext_ilmbase ext_zlib
)
else()
ExternalProject_Add(ext_openexr
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL http://files.kde.org/krita/build/dependencies/openexr-2.2.0.tar.gz
        URL_MD5 b64e931c82aa3790329c21418373db4e

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/openexr.diff

        INSTALL_DIR ${EXTPREFIX_openexr}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openexr} -DILMBASE_PACKAGE_PREFIX=${EXTPREFIX_openexr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DNAMESPACE_VERSIONING=OFF

        UPDATE_COMMAND ""
        ALWAYS 0
        DEPENDS ext_ilmbase ext_zlib
)
endif()
