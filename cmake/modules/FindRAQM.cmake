include(FindPkgConfig)
pkg_check_modules(PC_RAQM raqm)
find_path(RAQM_INCLUDE_DIRS NAMES raqm.h HINTS ${PC_RAQM_INCLUDE_DIRS}
  ${PC_RAQM_INCLUDEDIR})
find_library(RAQM_LIBRARIES NAMES raqm HINTS ${PC_RAQM_LIBRARY_DIRS}
  ${PC_RAQM_LIBDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RAQM DEFAULT_MSG RAQM_INCLUDE_DIRS RAQM_LIBRARIES)
