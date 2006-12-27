include_directories( ${KOFFICECORE_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LCMS_INCLUDE_DIR}  ${PIGMENT_INCLUDES} )

if(OPENEXR_FOUND)
include_directories(${OPENEXR_INCLUDE_DIR})
endif(OPENEXR_FOUND)


add_subdirectory( rgb_u8 )
add_subdirectory( gray_u8 )
add_subdirectory( gray_u16 )
add_subdirectory( cmyk_u8 )
add_subdirectory( cmyk_u16 )

