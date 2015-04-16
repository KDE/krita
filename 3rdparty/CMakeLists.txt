if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-unused-variable)
endif ()

if (SHOULD_BUILD_LIB_KDGANTT)
    add_subdirectory(kdgantt)
endif ()

if (SHOULD_BUILD_LIB_KDCHART)
    add_subdirectory(kdchart)
endif ()

if (WIN32 AND USE_BREAKPAD)
    add_subdirectory(google-breakpad)
endif ()

