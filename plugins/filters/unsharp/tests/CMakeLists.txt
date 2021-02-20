include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

ecm_add_tests(
    kis_unsharp_mask_test.cpp
    NAME_PREFIX "krita-filters-unsharp-"
    LINK_LIBRARIES kritaui Qt5::Test)
