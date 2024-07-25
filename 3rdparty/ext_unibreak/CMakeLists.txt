SET(EXTPREFIX_unibreak "${EXTPREFIX}" ) 

ExternalProject_Add(
    ext_unibreak
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/adah1972/libunibreak/releases/download/libunibreak_5_0/libunibreak-5.0.tar.gz
    URL_HASH SHA256=58f2fe4f9d9fc8277eb324075ba603479fa847a99a4b134ccb305ca42adf7158

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Add-CMake-toolchain.patch
    INSTALL_DIR ${EXTPREFIX_unibreak}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_unibreak} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
