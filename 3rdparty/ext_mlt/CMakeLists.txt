
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

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE} -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mlt}
)

set(DLFCN_WIN32_DEP ext_dlfcn_win32)
set(WIN32_CMAKE_ARGS -DWINDOWS_DEPLOY=OFF)
endif()

ExternalProject_Add(
  ext_mlt
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  GIT_REPOSITORY https://github.com/Eoin-ONeill-Yokai/mlt.git
  GIT_TAG origin/mlt-krita-plugins

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE} -DMOD_OPENCV=OFF ${WIN32_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mlt}
  DEPENDS ext_ffmpeg ${DLFCN_WIN32_DEP}
)
endif()
