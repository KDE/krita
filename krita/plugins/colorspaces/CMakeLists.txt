# Look for OpenCTL (XXX: move this inside full_pigment? Or inside pigment?)
macro_optional_find_package(OpenCTL)

macro_log_feature(OPENCTL_FOUND "OpenCTL" "Free Color Transformation Language implementation" "http://www.openctl.org" FALSE "0.9.2" "Required for High Dynamic Range Color Spaces, YCbCr and LMS support")

if(OPENCTL_FOUND)
  add_subdirectory(ctlcs)
endif(OPENCTL_FOUND)

add_subdirectory( gray_u8_no_alpha )
add_subdirectory( gray_u16_no_alpa )
add_subdirectory( xyz_u16 )
add_subdirectory( extensions )

