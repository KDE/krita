include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    KisQImageIOTest.cpp
    TEST_NAME KisQImageIOTest
    LINK_LIBRARIES kritaui kritatestsdk
    )
