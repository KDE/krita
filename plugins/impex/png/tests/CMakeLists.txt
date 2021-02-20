include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()
include(KritaAddBrokenUnitTest)

krita_add_broken_unit_test(kis_png_test.cpp
    TEST_NAME kis_png_test
    LINK_LIBRARIES kritaui Qt5::Test
    NAME_PREFIX "plugins-impex-")
