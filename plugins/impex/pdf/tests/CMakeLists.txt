include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    KisPdfTest.cpp
    TEST_NAME KisPdfTest
    LINK_LIBRARIES kritaui kritatestsdk
    )
