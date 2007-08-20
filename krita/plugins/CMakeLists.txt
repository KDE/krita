add_subdirectory( filters )
add_subdirectory( paintops )
add_subdirectory( tools )
add_subdirectory( viewplugins )

find_package(GLPK)

if(GLPK_FOUND)
    add_subdirectory( painterlyframework )
else(GLPK_FOUND)
    message(STATUS "Could not find the GLPK library. The painterly mixer plugin will not be built.")
endif(GLPK_FOUND)
