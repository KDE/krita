macro_add_unittest_definitions()

kis_add_test(
    KisOraTest.cpp
    TEST_NAME KisOraTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )

