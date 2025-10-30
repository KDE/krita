include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    KisGifTest.cpp
    TEST_NAME KisGifTest
    LINK_LIBRARIES kritaui kritatestsdk
    )
