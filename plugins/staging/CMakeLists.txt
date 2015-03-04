if(SHOULD_BUILD_PLUGIN_GOOGLEDOCS)
#    add_subdirectory(googledocs)
endif()

if (SHOULD_BUILD_PLUGIN_THREEDSHAPE)
    add_subdirectory(threedshape)
endif ()

if(SHOULD_BUILD_PLUGIN_SHEETS_PIVOTTABLES)
    add_subdirectory(pivottables)
endif()
