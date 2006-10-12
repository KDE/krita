include_directories( ${KOFFICECORE_INCLUDES} )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

# the core library
add_subdirectory( core )

# optional modules
add_subdirectory( modules )

# commandline-prog to run scripts
add_subdirectory( runner )

# unittests and such stuff
if (KDE4_BUILD_TESTS)
	add_subdirectory( test )
endif (KDE4_BUILD_TESTS)

# KDE Javascript
# test kjsembed I think
add_subdirectory( kjs )

# Python
MESSAGE(STATUS "s :PYTHON_INCLUDE_PATH :<${PYTHON_INCLUDE_PATH}")
if(PYTHON_INCLUDE_PATH)
	add_subdirectory(python)
endif(PYTHON_INCLUDE_PATH)		

# Ruby

# temporary to work around a bug in the FindRUBY.cmake as deployed with cmake.
#macro_optional_find_package(RUBY)
set(MyRuby_DIR ${CMAKE_SOURCE_DIR}/libs/kross )
find_package(MyRuby)

if(RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
	add_subdirectory(ruby)
endif(RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
