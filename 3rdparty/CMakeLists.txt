if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
    add_definitions(-Wno-unused-variable)
endif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)

if (SHOULD_BUILD_LIB_KDGANTT)
    add_subdirectory(kdgantt)
endif (SHOULD_BUILD_LIB_KDGANTT)

if (SHOULD_BUILD_LIB_KDCHART)
    add_subdirectory(kdchart)
endif (SHOULD_BUILD_LIB_KDCHART)
