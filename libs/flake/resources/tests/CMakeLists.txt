include(ECMAddTests)

include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

macro_add_unittest_definitions()

# needs kritaimage for TestUtil
if(APPLE)
    # Broken unit test.app can be run if installed
    krita_add_broken_unit_tests(
        KoGamutMaskTest.cpp
        LINK_LIBRARIES kritaflake kritaimage Qt5::Test
        NAME_PREFIX "libs-flake-"
        TARGET_NAMES_VAR BROKEN_TESTS
        GUI)

    macos_test_fixrpath(${BROKEN_TESTS})


else(APPLE)
    ecm_add_tests(
        KoGamutMaskTest.cpp
        LINK_LIBRARIES kritaflake kritaimage Qt5::Test
        NAME_PREFIX "libs-flake-")
endif()


