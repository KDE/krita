SET(PREFIX_ext_mlt "${EXTPREFIX}" )

ExternalProject_Add( ext_mlt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY git@github.com:Eoin-ONeill-Yokai/mlt.git 
    GIT_TAG origin/mlt-krita-plugins
  
    INSTALL_DIR ${PREFIX_ext_mlt}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_mlt}  -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
)
