include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    KisRawTest.cpp
    TEST_NAME KisRawTest
    LINK_LIBRARIES kritaui kritatestsdk
    )
