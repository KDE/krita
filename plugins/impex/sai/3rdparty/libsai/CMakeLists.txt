cmake_minimum_required( VERSION 3.8.0 )
project( sai )

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

if( MSVC )
	add_compile_options( /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	add_compile_options( -march=native )
	add_compile_options( -Wall )
	add_compile_options( -Wextra )
endif()

### libsai
add_library(
	sai
	source/sai.cpp
)
target_include_directories(
	sai
	PUBLIC
	include
)


### Decryption sample
add_executable(
	Decrypt
	samples/Decrypt.cpp
)

target_link_libraries(
	Decrypt
	PRIVATE
	sai
)

### VFS tree sample
add_executable(
	Tree
	samples/Tree.cpp
)

target_link_libraries(
	Tree
	PRIVATE
	sai
)
