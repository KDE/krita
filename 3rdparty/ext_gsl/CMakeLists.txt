#
# From https://github.com/ampl/gsl. which adds CMake support to gsl
#
SET(EXTPREFIX_gsl "${EXTPREFIX}" )
if (MSVC)
ExternalProject_Add(
    ext_gsl
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    GIT_REPOSITORY https://github.com/ampl/gsl.git
    GIT_TAG 709cc572279e4a56b0e218b834f202c1b3f757af
    INSTALL_DIR ${EXTPREFIX_gsl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_gsl} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE}
    UPDATE_COMMAND ""
    ALWAYS 0
)
else()
ExternalProject_Add( ext_gsl

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://files.kde.org/krita/build/dependencies/gsl-2.1.tar.gz
    URL_MD5 625f4105fcf653822a96294c1b801ad4

    INSTALL_DIR ${EXTPREFIX_gsl}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_gsl} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DGSL_SHARED=ON
    UPDATE_COMMAND ""
    ALWAYS 0
)
endif()
