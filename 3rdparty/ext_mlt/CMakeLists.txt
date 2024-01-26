
SET(PREFIX_ext_mlt "${EXTPREFIX}")

if (NOT WIN32 OR MINGW)
set(DLFCN_WIN32_DEP)
set(WIN32_CMAKE_ARGS)

if (WIN32)

  ExternalProject_Add(
    ext_dlfcn_win32
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/dlfcn-win32/dlfcn-win32/archive/refs/tags/v1.3.1.zip
    URL_HASH SHA256=4f611c4372eef7f0179a33f76f84d54857c4fe676b60b654c6c5d91a6d4dad55

    CMAKE_ARGS ${GLOBAL_PROFILE} -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mlt}
  )

  set(DLFCN_WIN32_DEP ext_dlfcn_win32)
  set(WIN32_CMAKE_ARGS -DWINDOWS_DEPLOY=OFF)
endif()

if (ANDROID)
  set(BUILD_EXTRA_OPTS "-DMOD_RTAUDIO=OFF")
else()
  set(BUILD_EXTRA_OPTS "")
endif()



kis_ExternalProject_Add_with_separate_builds_apple(
  ext_mlt
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  URL https://github.com/mltframework/mlt/releases/download/v7.22.0/mlt-7.22.0.tar.gz
  URL_HASH MD5=9f747ae1b7388937be6328b4c234431b
  #GIT_REPOSITORY https://github.com/emmetoneillpdx/mlt.git
  #GIT_TAG origin/master
  # GIT_TAG 1924f3712e3dc0f7a1e5a1aa658dbbbf0e863754 # fixed tip of 'origin/mlt-krita-plugins'

  PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-Fix-compilation-on-Windows-with-Clang-15-965.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0001-macOS-allow-finding-plugins-outside-bundles.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/0002-MLT-check-pointers-before-dereferencing.patch

  CMAKE_ARGS ${GLOBAL_PROFILE} -DMOD_OPENCV=OFF ${WIN32_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mlt} ${BUILD_EXTRA_OPTS}

  DEPENDS ext_ffmpeg ${DLFCN_WIN32_DEP}
)
endif()
