add_subdirectory(barcode)
add_subdirectory(chart)

#an unfinished plugin
#add_subdirectory(shape)

if(MARBLE_FOUND)
  macro_optional_add_subdirectory(maps)
endif(MARBLE_FOUND)
