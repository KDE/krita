macro_add_unittest_definitions()

kis_add_test(
    KisQImageIOTest.cpp
    TEST_NAME KisQImageIOTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )
