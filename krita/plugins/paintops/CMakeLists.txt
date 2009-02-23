include_directories(libpaintop)

add_subdirectory( libpaintop )
add_subdirectory( defaultpaintops )

add_subdirectory( sumie )
add_subdirectory( chalk )
add_subdirectory( filterop )
add_subdirectory( complexop )
add_subdirectory( dynamicbrush )
#add_subdirectory( cpaint )

if( BUILD_EVERYTHING )
    add_subdirectory( dynadraw )
    add_subdirectory( deform )
    add_subdirectory( curvebrush )
    add_subdirectory( spray )
endif( BUILD_EVERYTHING )

