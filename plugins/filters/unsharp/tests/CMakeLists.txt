macro_add_unittest_definitions()

kis_add_tests(
    kis_unsharp_mask_test.cpp
    NAME_PREFIX "krita-filters-unsharp-"
    LINK_LIBRARIES kritaui kritatestsdk)

