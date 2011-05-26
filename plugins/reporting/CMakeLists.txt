add_subdirectory(barcode)
add_subdirectory(chart)

#an unfinished plugin
add_subdirectory(shape)

find_package(Marble)

if(MARBLE_FOUND)
  add_subdirectory(maps)
else(MARBLE_FOUND)
  message(WARNING "No marble found - There is on marble in system - reporting plugin not builded.")
endif(MARBLE_FOUND)
