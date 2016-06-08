if (SHOULD_BUILD_PLUGIN_ARTISTICTEXTSHAPE)
    add_subdirectory( artistictextshape )
endif ()

if (SHOULD_BUILD_PLUGIN_PATHSHAPES)
    add_subdirectory( pathshapes )
endif ()

if (SHOULD_BUILD_PLUGIN_TEXTSHAPE)
    add_subdirectory( textshape )
endif()

if (SHOULD_BUILD_PLUGIN_VECTORSHAPE)
    add_subdirectory( vectorshape )
endif ()
