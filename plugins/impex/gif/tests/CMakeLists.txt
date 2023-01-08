macro_add_unittest_definitions()

kis_add_test(
    KisGifTest.cpp
    TEST_NAME KisGifTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )
