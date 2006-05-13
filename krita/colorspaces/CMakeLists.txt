

add_subdirectory( rgb_u8 )
add_subdirectory( rgb_u16 )
add_subdirectory( rgb_f32 )
add_subdirectory( gray_u8 )
add_subdirectory( gray_u16 )
add_subdirectory( cmyk_u8 )
add_subdirectory( cmyk_u16 )
add_subdirectory( lms_f32 )
add_subdirectory( wet )

if(OPENEXR_FOUND)
	add_subdirectory( rgb_f16half )
endif(OPENEXR_FOUND)



########### install files ###############






#original Makefile.am contents follow:

## Definition of the service type
#
#if have_openexr
#HALF_SUBDIRS=rgb_f16half
#endif
#
#SUBDIRS = rgb_u8 rgb_u16 rgb_f32 \
#	gray_u8 gray_u16 \
#	cmyk_u8 cmyk_u16 \
#	lms_f32 \
#	$(HALF_SUBDIRS) \
#	wet
##wet wetsticky
#
