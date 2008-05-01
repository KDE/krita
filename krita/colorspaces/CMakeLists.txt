
if(OPENCTL_FOUND)
  add_subdirectory( lms_f32 )
  add_subdirectory( xyz_float_hdr.ctl )
  add_subdirectory( rgb_float_hdr.ctl )
  add_subdirectory( ycbcr )
else(OPENCTL_FOUND)
  add_subdirectory( rgb_float_hdr )
endif(OPENCTL_FOUND)

add_subdirectory( gray_u8_no_alpha )
add_subdirectory( gray_u16_no_alpa )
add_subdirectory( xyz_u16 )
add_subdirectory( extensions )
