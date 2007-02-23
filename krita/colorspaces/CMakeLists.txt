#add_subdirectory( rgb_f32 )
#add_subdirectory( lms_f32 )
#add_subdirectory( wet )

MESSAGE(STATUS "Remember to port ycbcr_u16 and ycbcr_u8 and all the rest when Cyrille has finished his templated design")
#add_subdirectory(ycbcr_u16)
#add_subdirectory(ycbcr_u8)

add_subdirectory( rgb_float_hdr )
add_subdirectory( gray_u8 )
add_subdirectory( gray_u16 )
add_subdirectory( xyz_u16 )

