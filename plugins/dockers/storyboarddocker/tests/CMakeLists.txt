include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/.. 
    ${CMAKE_SOURCE_DIR}/libs/ui/tests 
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include(ECMAddTests)

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()


kis_add_tests(
    StoryboardModelTest.cpp
    StoryboardTimelineSyncTest.cpp
    NAME_PREFIX "plugins-dockers-storyboarddocker-"
    LINK_LIBRARIES kritastoryboarddocker_static kritaui Qt5::Test
    )
