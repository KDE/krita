add_subdirectory( filters )
add_subdirectory( paintops )
add_subdirectory( tools )
add_subdirectory( viewplugins )

find_package(GLPK)
set(REQUIRED_GLPK_VERSION 419)

if(GLPK_FOUND AND NOT GLPK_VERSION LESS ${REQUIRED_GLPK_VERSION})
    set(HAVE_REQUIRED_GLPK_VERSION TRUE)
else(GLPK_FOUND AND NOT GLPK_VERSION LESS ${REQUIRED_GLPK_VERSION})
    set(HAVE_REQUIRED_GLPK_VERSION FALSE)
endif(GLPK_FOUND AND NOT GLPK_VERSION LESS ${REQUIRED_GLPK_VERSION})

macro_log_feature(HAVE_REQUIRED_GLPK_VERSION "GLPK" "The Krita painterly mixer plugin will not be built" "http://www.gnu.org/software/glpk" FALSE "4.19")

if(HAVE_REQUIRED_GLPK_VERSION)
    add_subdirectory( painterlyframework )
endif(HAVE_REQUIRED_GLPK_VERSION)

