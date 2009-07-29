if(MSVC)
  # avoid "cannot open file 'LIBC.lib'" error
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC.LIB")
endif(MSVC)


include_directories(
        ${KOMAIN_INCLUDES}
)

IF (NOT TINY)
    macro_optional_add_subdirectory( scan )
    macro_optional_add_subdirectory( colorspaces )
    macro_optional_add_subdirectory( paragraphtool )
    macro_optional_add_subdirectory( musicshape )
    macro_optional_add_subdirectory( divineProportion )

    macro_optional_find_package(Spnav)
    macro_log_feature(SPNAV_FOUND "Spnav" "3Dconnexion device driver and SDK" "http://spacenav.sourceforge.net/" FALSE "" "Required by SpaceNavigator 3D mouse plugin")

    IF (SPNAV_FOUND)
        macro_optional_add_subdirectory( spacenavigator )
    ENDIF (SPNAV_FOUND)
    macro_optional_add_subdirectory( artistictextshape )
    macro_optional_add_subdirectory( simpletextedit )
    macro_optional_add_subdirectory( dockers )
    macro_optional_add_subdirectory( textediting )
ENDIF (NOT TINY)

add_subdirectory( variables )
add_subdirectory( defaultTools )
add_subdirectory( pathshapes )
add_subdirectory( pictureshape )
add_subdirectory( textshape )

