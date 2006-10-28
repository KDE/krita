include_directories( ${KOFFICECORE_INCLUDES} )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

# the core library
add_subdirectory( core )

# optional modules
add_subdirectory( modules )

# commandline-prog to run scripts
add_subdirectory( console )

# unittests and such stuff
if (KDE4_BUILD_TESTS)
	add_subdirectory( test )
endif (KDE4_BUILD_TESTS)

# KDE Javascript
# test kjsembed I think
add_subdirectory( kjs )

# Python
#message(STATUS "s :PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")
#message(STATUS "s :PYTHON_INCLUDE_PATH: ${PYTHON_INCLUDE_PATH}")
if(PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
	add_subdirectory(python)
endif(PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)

# Ruby
macro_optional_find_package(RUBY)
message(STATUS "s :RUBY_LIBRARY: ${RUBY_LIBRARY}")
message(STATUS "s :RUBY_INCLUDE_PATH: ${RUBY_INCLUDE_PATH}")
if(RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
	add_subdirectory(ruby)
endif(RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
