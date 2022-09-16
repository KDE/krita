
SET(PREFIX_ext_mlt "${EXTPREFIX}")

ExternalProject_Add(
  ext_mlt
  DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
  GIT_REPOSITORY https://github.com/Eoin-ONeill-Yokai/mlt.git
  GIT_TAG origin/mlt-krita-plugins

  CMAKE_ARGS ${CMAKE_GLOBAL_PROFILE} -DMOD_OPENCV=OFF
  BUILD_IN_SOURCE 1
  BUILD_COMMAND cmake --build .
  INSTALL_COMMAND cmake --install . --prefix ${PREFIX_ext_mlt}
)
