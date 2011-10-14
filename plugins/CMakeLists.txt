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
    macro_optional_add_subdirectory( colorengines )
    #macro_optional_add_subdirectory( paragraphtool )
    macro_optional_add_subdirectory( musicshape )

    macro_optional_find_package(Spnav)
    macro_log_feature(SPNAV_FOUND "Spnav" "3Dconnexion device driver and SDK" "http://spacenav.sourceforge.net/" FALSE "" "Required by SpaceNavigator 3D mouse plugin")

    IF (SPNAV_FOUND)
        macro_optional_add_subdirectory( spacenavigator )
    ENDIF (SPNAV_FOUND)
    macro_optional_add_subdirectory( artistictextshape )
    macro_optional_add_subdirectory( dockers )
    macro_optional_add_subdirectory( textediting )
    macro_optional_add_subdirectory( commentshape )
    macro_optional_add_subdirectory( staging )
    if(MARBLE_FOUND)
        macro_optional_add_subdirectory( mapshape )
    endif(MARBLE_FOUND)
    if(SHOULD_BUILD_KEXI)
        macro_optional_add_subdirectory( kexi )
    endif(SHOULD_BUILD_KEXI)
ENDIF (NOT TINY)
macro_optional_add_subdirectory( variables )
macro_optional_add_subdirectory( defaultTools )
macro_optional_add_subdirectory( pathshapes )
macro_optional_add_subdirectory( chartshape )
macro_optional_add_subdirectory( pictureshape )
macro_optional_add_subdirectory( videoshape )
macro_optional_add_subdirectory( textshape )
macro_optional_add_subdirectory( pluginshape )
macro_optional_add_subdirectory( treeshape )
macro_optional_add_subdirectory( formulashape )

# as the vector shape depends on filters subdir (not really intuitive) lets allow disabling
macro_optional_add_subdirectory( vectorshape )

# only build the reporting plugins if building koreport
if(SHOULD_BUILD_KOREPORT)
    macro_optional_add_subdirectory( reporting )
endif(SHOULD_BUILD_KOREPORT)
