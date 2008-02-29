add_subdirectory( filters )
add_subdirectory( paintops )
add_subdirectory( tools )
add_subdirectory( viewplugins )
#add_subdirectory( brushes )
add_subdirectory( generators )

find_package(GSL)

if( GSL_FOUND )
    add_subdirectory( painterlyframework )
endif( GSL_FOUND )
