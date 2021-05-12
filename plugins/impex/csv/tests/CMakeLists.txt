include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

macro_add_unittest_definitions()
include(KritaAddBrokenUnitTest)

krita_add_broken_unit_test(
    kis_csv_test.cpp
    TEST_NAME kis_csv_test
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-"
    ${MACOS_GUI_TEST}
    )

macos_test_fixrpath(kis_csv_test)
