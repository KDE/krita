include_directories(libpaintop)

add_subdirectory( libpaintop )
add_subdirectory( defaultpaintops )
add_subdirectory( sumie )
add_subdirectory( chalk )
add_subdirectory( filterop )
add_subdirectory( complexop )
add_subdirectory( dynamicbrush )
#add_subdirectory( cpaint )

    add_subdirectory( dynadraw )
    add_subdirectory( deform )
    add_subdirectory( curvebrush )
	add_subdirectory( spray )

add_subdirectory( dynamicbrush )

if( BUILD_EVERYTHING )
endif( BUILD_EVERYTHING )

