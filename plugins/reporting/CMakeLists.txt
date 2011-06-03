add_subdirectory(barcode)
add_subdirectory(chart)

#an unfinished plugin
add_subdirectory(shape)

if(MARBLE_FOUND)
  add_subdirectory(maps)
endif(MARBLE_FOUND)
