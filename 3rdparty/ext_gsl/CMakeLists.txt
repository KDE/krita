#
# From https://github.com/ampl/gsl. which adds CMake support to gsl
#
SET(EXTPREFIX_gsl "${EXTPREFIX}" )
ExternalProject_Add( ext_gsl

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/gsl-2.1.tar.gz
    URL_MD5 625f4105fcf653822a96294c1b801ad4

    INSTALL_DIR ${EXTPREFIX_gsl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_gsl} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DGSL_SHARED=ON
    UPDATE_COMMAND ""
    ALWAYS 0
)
