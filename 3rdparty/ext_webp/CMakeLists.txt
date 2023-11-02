SET(EXTPREFIX_webp "${EXTPREFIX}" )

kis_ExternalProject_Add_with_separate_builds_apple(
    ext_webp
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-1.3.2.tar.gz
    URL_HASH SHA256=2a499607df669e40258e53d0ade8035ba4ec0175244869d1025d460562aa09b4

    PATCH_COMMAND  ${PATCH_COMMAND}  -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0003-fix-tool-dependencies.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0007-fix-arm-build.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0008-sdl.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_webp} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DBUILD_SHARED_LIBS=ON ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
)
