macro_add_unittest_definitions()

krita_add_broken_unit_test(
    kis_csv_test.cpp
    TEST_NAME kis_csv_test
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )
