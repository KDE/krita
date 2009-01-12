include_directories(libpaintop)

add_subdirectory( libpaintop )
add_subdirectory( defaultpaintops )
add_subdirectory( sumie )
add_subdirectory( chalk )
add_subdirectory( filterop )
add_subdirectory( complexop )
#add_subdirectory( cpaint )

if( BUILD_EVERYTHING )
    add_subdirectory( deform )
    add_subdirectory( curvebrush )
endif( BUILD_EVERYTHING )

if( NOT MSVC )
    #add_subdirectory( dynamicbrush )
endif( NOT MSVC )
