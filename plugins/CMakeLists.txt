if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif()

# add all the include paths to kritaui that we use, for now.

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
    ${Vc_INCLUDE_DIR}
)

add_subdirectory( assistants )
add_subdirectory( color )
add_subdirectory( dockers )
add_subdirectory( extensions )
add_subdirectory( filters )
add_subdirectory( flake )
add_subdirectory( generators )
add_subdirectory( impex )
add_subdirectory( metadata )
add_subdirectory( paintops )
add_subdirectory( tools )
add_subdirectory( qt )

if (HAVE_PYQT5 AND HAVE_SIP AND HAVE_PYTHONLIBS)
    add_subdirectory( python )
endif()
