include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/.. 
    ${CMAKE_SOURCE_DIR}/libs/ui/tests 
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
include(ECMAddTests)

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()

if (APPLE)

    ecm_add_tests(
        StoryboardModelTest.cpp

        NAME_PREFIX "plugins-dockers-storyboarddocker-"
        LINK_LIBRARIES kritastoryboarddocker kritaui Qt5::Test
        TARGET_NAMES_VAR OK_TESTS
        ${MACOS_GUI_TEST}
        )

    # Some broken unit test.app can be run if installed
    krita_add_broken_unit_tests(
        StoryboardTimelineSyncTest.cpp

        NAME_PREFIX "plugins-dockers-storyboarddocker-"
        LINK_LIBRARIES kritastoryboarddocker kritaui Qt5::Test
        TARGET_NAMES_VAR BROKEN_TESTS
        ${MACOS_GUI_TEST})


macos_test_fixrpath(${OK_TESTS} ${BROKEN_TESTS})

else (APPLE)

    ecm_add_tests(
        StoryboardModelTest.cpp
        StoryboardTimelineSyncTest.cpp
        NAME_PREFIX "plugins-dockers-storyboarddocker-"
        LINK_LIBRARIES kritastoryboarddocker kritaui Qt5::Test
        )

endif()
