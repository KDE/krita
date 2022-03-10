SET(EXTPREFIX_webp "${EXTPREFIX}" )

ExternalProject_Add( ext_webp
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-1.2.2.tar.gz
    URL_HASH SHA256=7656532f837af5f4cec3ff6bafe552c044dc39bf453587bd5b77450802f4aee6

    PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-CMake-targets-install-directory.patch

    # Google specifies types for most of its library components
    # forcing a shared lib makes it not export a .lib on MSVC
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_webp} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
