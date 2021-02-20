include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/.. 
    ${CMAKE_SOURCE_DIR}/libs/ui/tests 
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

include(ECMAddTests)

macro_add_unittest_definitions()

ecm_add_tests(
    StoryboardModelTest.cpp
    StoryboardTimelineSyncTest.cpp

    NAME_PREFIX "plugins-dockers-storyboarddocker-"
    LINK_LIBRARIES kritastoryboarddocker kritaui Qt5::Test
)
