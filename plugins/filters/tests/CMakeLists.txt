macro_add_unittest_definitions()
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

include(KritaAddBrokenUnitTest)

krita_add_broken_unit_tests(
    kis_all_filter_test.cpp
    kis_crash_filter_test.cpp

    NAME_PREFIX "krita-filters-"
    LINK_LIBRARIES kritaimage Qt5::Test
    TARGET_NAMES_VAR BROKEN_TESTS
    ${MACOS_GUI_TEST}
    )

macos_test_fixrpath(${BROKEN_TESTS})
