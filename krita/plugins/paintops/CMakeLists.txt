add_subdirectory( defaultpaintops )
add_subdirectory( smearybrush )
add_subdirectory( cpaint )

if(WIN32)
    message(STATUS "Fix dynamicbrush compile for win32")
else(WIN32)
    add_subdirectory( dynamicbrush )
endif(WIN32)

add_subdirectory( filterop )
