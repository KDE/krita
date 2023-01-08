macro_add_unittest_definitions()

kis_add_test(KisJP2Test.cpp
    TEST_NAME KisJP2Test
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-impex-"
)

