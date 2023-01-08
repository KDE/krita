macro_add_unittest_definitions()

kis_add_test(
    KisQmlTest.cpp
    TEST_NAME KisQmlTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )

