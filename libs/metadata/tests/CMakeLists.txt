include(ECMAddTests)
include(KritaAddBrokenUnitTest)
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)
macro_add_unittest_definitions()

if (APPLE)
    # Broken unit test.app can be run if installed
    krita_add_broken_unit_tests(
        kis_meta_data_test.cpp
        NAME_PREFIX "libs-metadata-"
        LINK_LIBRARIES kritametadata Qt5::Test
        TARGET_NAMES_VAR OK_TESTS
        GUI)

    macos_test_fixrpath(${OK_TESTS})

else (APPLE)

    ecm_add_tests(
        kis_meta_data_test.cpp
        NAME_PREFIX "libs-metadata-"
        LINK_LIBRARIES kritametadata Qt5::Test )


endif()

