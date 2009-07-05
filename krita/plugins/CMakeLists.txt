# Look for OpenCTL , this is needed here for openexr
macro_optional_find_package(OpenCTL)

macro_log_feature(OPENCTL_FOUND "OpenCTL" "Free Color Transformation Language implementation" "http://www.openctl.org" FALSE "0.9.2" "Required for High Dynamic Range Color Spaces, YCbCr and LMS support")

add_subdirectory( assistants )
add_subdirectory( filters )
add_subdirectory( tools )
add_subdirectory( extensions )
add_subdirectory( generators )
add_subdirectory( paintops )
add_subdirectory( formats )
add_subdirectory( colorspaces )
