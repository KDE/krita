krita_add_broken_unit_tests(
    kis_all_filter_test.cpp
    kis_crash_filter_test.cpp

    NAME_PREFIX "krita-filters-"
    LINK_LIBRARIES kritaimage kritatestsdk
    )
