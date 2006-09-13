add_subdirectory( rgb_u8 )
#add_subdirectory( rgb_u16 )
#add_subdirectory( rgb_f32 )
#add_subdirectory( gray_u8 )
#add_subdirectory( gray_u16 )
#add_subdirectory( cmyk_u8 )
#add_subdirectory( cmyk_u16 )
#add_subdirectory( lms_f32 )
#add_subdirectory( wet )

MESSAGE(STATUS "Remember to port ycbcr_u16 and ycbcr_u8")
#add_subdirectory(ycbcr_u16)
#add_subdirectory(ycbcr_u8)

if(OPENEXR_FOUND)
    #	add_subdirectory( rgb_f16half )
endif(OPENEXR_FOUND)

