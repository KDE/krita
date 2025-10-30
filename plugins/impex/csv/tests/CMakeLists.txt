include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

krita_add_broken_unit_test(
    kis_csv_test.cpp
    TEST_NAME kis_csv_test
    LINK_LIBRARIES kritaui kritatestsdk
    )
