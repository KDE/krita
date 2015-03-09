find_path(SPNAV_INCLUDE_DIR spnav.h )

find_library(SPNAV_LIBRARY NAMES spnav ) 

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Spnav DEFAULT_MSG SPNAV_INCLUDE_DIR SPNAV_LIBRARY )
