SET(EXTPREFIX_libunwindstack-ndk "${EXTPREFIX}")

ExternalProject_Add(
    ext_libunwindstack-ndk
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    SOURCE_SUBDIR ./cmake
    GIT_REPOSITORY https://github.com/getsentry/libunwindstack-ndk.git
    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-install-property-and-cmake-config-file.patch

    INSTALL_DIR ${EXTPREFIX_libunwindstack-ndk}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_libunwindstack-ndk}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
                ${GLOBAL_PROFILE}
    UPDATE_COMMAND ""
)
