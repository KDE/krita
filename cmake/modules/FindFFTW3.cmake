
INCLUDE(FindPkgConfig)

pkg_check_modules(FFTW3 fftw3>=3.2)

if (FFTW3_FOUND)
    message(STATUS "FFTW Found Version: " ${FFTW_VERSION})
endif()
