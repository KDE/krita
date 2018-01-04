#
# From https://github.com/ampl/gsl. which adds CMake support to gsl
#
SET(EXTPREFIX_gsl "${EXTPREFIX}" )
ExternalProject_Add( ext_gsl

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/gsl-2.3.0.tar.gz
    URL_MD5 7e0478f7c5e62696fef480b9a46f708c

    INSTALL_DIR ${EXTPREFIX_gsl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_gsl} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DGSL_SHARED=ON -DBUILD_TESTING=OFF
    UPDATE_COMMAND ""
)
