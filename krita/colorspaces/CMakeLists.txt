
if(OPENCTL_FOUND)
    add_subdirectory( lms_f32 )
    add_subdirectory( xyz_float_hdr.ctl )
else(OPENCTL_FOUND)
# Disabled, since the change to pigment breaks it, and that it will soon
# be replace by the CTL version
#     add_subdirectory( xyz_float_hdr )
endif(OPENCTL_FOUND)

add_subdirectory( rgb_float_hdr )
add_subdirectory( gray_u8_no_alpha )
add_subdirectory( gray_u16_no_alpa )
add_subdirectory( xyz_u16 )
add_subdirectory( ycbcr )
add_subdirectory( extensions )
