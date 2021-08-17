SET(EXTPREFIX_webp "${EXTPREFIX}" )

ExternalProject_Add( ext_webp
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-1.2.0.tar.gz
    URL_HASH SHA512=c46b41899a543cc80914c89646dd607dbb5d025a9727dd83ef70994b9310eedc697666bd812141a90aa16632a9a354a031d9360a9ee1112295c7e154e69f0b74

    PATCH_COMMAND  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-CMake-targets-install-directory.patch

    # Google specifies types for most of its library components
    # forcing a shared lib makes it not export a .lib on MSVC
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_webp} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_SHARED_LIBS=OFF ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
