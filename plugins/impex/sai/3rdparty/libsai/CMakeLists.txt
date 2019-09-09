cmake_minimum_required( VERSION 3.8.0 )
project( sai )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

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


if( MSVC )
	target_compile_options( sai PUBLIC /W3 )
elseif( CMAKE_COMPILER_IS_GNUCXX )
	target_compile_options( sai PUBLIC -Wall )
	target_compile_options( sai PUBLIC -Wextra )
endif()


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

### VFS tree sample
add_executable(
	Document
	samples/Document.cpp
)

target_link_libraries(
	Document
	PRIVATE
	sai
)
