if (SHOULD_BUILD_PLAN)
    macro_optional_add_subdirectory(kdgantt)
endif (SHOULD_BUILD_PLAN)

if (NOT CREATIVEONLY)
    add_subdirectory(kdchart)
endif (NOT CREATIVEONLY)
