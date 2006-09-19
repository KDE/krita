# temporary to work around a bug in the FindRUBY.cmake as deployed with cmake.
#macro_optional_find_package(RUBY)

set(MyRuby_DIR ${CMAKE_SOURCE_DIR}/libs/kross )
find_package(MyRuby)

include_directories( ${KOFFICECORE_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory( core )
#add_subdirectory( runner )
add_subdirectory( test )

# KDE Javascript
# test kjsembed I think
add_subdirectory( kjs )

# Python
MESSAGE(STATUS "s :PYTHON_INCLUDE_PATH :<${PYTHON_INCLUDE_PATH}")
if(PYTHON_INCLUDE_PATH)
	add_subdirectory(python)
endif(PYTHON_INCLUDE_PATH)		

# Ruby
##MESSAGE(STATUS "Remember to fix ruby's configuration")
MESSAGE(STATUS "s :RUBY_INCLUDE_PATH :<${RUBY_INCLUDE_PATH}")
MESSAGE(STATUS "s :RUBY_EXECUTABLE :<${RUBY_EXECUTABLE}")
MESSAGE(STATUS "s :RUBY_LIBRARY :<${RUBY_LIBRARY}")
if(RUBY_INCLUDE_PATH AND RUBY_EXECUTABLE AND RUBY_LIBRARY)
	add_subdirectory(ruby)
endif(RUBY_INCLUDE_PATH AND RUBY_EXECUTABLE AND RUBY_LIBRARY)
