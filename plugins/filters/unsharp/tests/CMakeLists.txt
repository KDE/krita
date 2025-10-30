include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "krita-filters-unsharp-")

kis_add_tests(
    kis_unsharp_mask_test.cpp
    LINK_LIBRARIES kritaui kritatestsdk)

