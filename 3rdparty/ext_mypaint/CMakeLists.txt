    SET(PREFIX_ext_mypaint "${EXTPREFIX}" )
ExternalProject_Add( ext_mypaint
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/mypaint/libmypaint/releases/download/v1.6.1/libmypaint-1.6.1.tar.xz
    URL_MD5 7f1dab2d30ce8a3f494354c7c77a2977
        
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-cmake-build-system.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-cmake-library-versioning-info-for-pkg-config.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-cmake-library-versioning-info-for-CMake-itself.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0004-cmake-Fix-relative-paths.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0005-Export-all-symbols.patch

    INSTALL_DIR ${PREFIX_ext_mypaint}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mypaint} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    DEPENDS ext_json_c
    UPDATE_COMMAND ""
)
