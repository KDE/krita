
if(OPENCTL_FOUND AND OPENEXR_FOUND)
  add_subdirectory(ctlcs)
endif(OPENCTL_FOUND AND OPENEXR_FOUND)

add_subdirectory( gray_u8_no_alpha )
add_subdirectory( gray_u16_no_alpa )
add_subdirectory( xyz_u16 )
add_subdirectory( extensions )

