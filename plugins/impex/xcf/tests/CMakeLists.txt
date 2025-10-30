include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    kis_xcf_test.cpp
    TEST_NAME kis_xcf_test
    LINK_LIBRARIES kritaui kritatestsdk
    )

