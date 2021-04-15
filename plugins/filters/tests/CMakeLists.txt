macro_add_unittest_definitions()
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

include(KritaAddBrokenUnitTest)

krita_add_broken_unit_test( kis_all_filter_test.cpp
    TEST_NAME kis_all_filter_test.cpp
    NAME_PREFIX "krita-filters-"
    LINK_LIBRARIES kritaimage Qt5::Test)
    
krita_add_broken_unit_test( kis_crash_filter_test.cpp
    TEST_NAME kis_crash_filter_test.cpp
    NAME_PREFIX "krita-filters-"
    LINK_LIBRARIES kritaimage Qt5::Test)
