if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()


if (SHOULD_BUILD_PLUGIN_COLORENGINES)
    add_subdirectory( colorengines )
endif ()

if (SHOULD_BUILD_PLUGIN_DOCKERS)
    add_subdirectory( dockers )
endif ()

add_subdirectory( tools )
add_subdirectory( flake )
