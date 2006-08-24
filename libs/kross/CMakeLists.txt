macro_optional_find_package(RUBY)
include_directories( ${KOFFICECORE_INCLUDES} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory( core )
#add_subdirectory( runner )
add_subdirectory( test )

# KDE Javascript
add_subdirectory( kjs )

# Python
MESSAGE(STATUS "s :PYTHON_INCLUDE_PATH :<${PYTHON_INCLUDE_PATH}")
if(PYTHON_INCLUDE_PATH)
	add_subdirectory(python)
endif(PYTHON_INCLUDE_PATH)		

# Ruby
MESSAGE(STATUS "s :RUBY_INCLUDE_PATH :<${RUBY_INCLUDE_PATH}")
MESSAGE(STATUS "s :RUBY_EXECUTABLE :<${RUBY_EXECUTABLE}")
if(RUBY_INCLUDE_PATH)
	add_subdirectory(ruby)
endif(RUBY_INCLUDE_PATH)
