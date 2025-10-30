include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(KisJP2Test.cpp
    TEST_NAME KisJP2Test
    LINK_LIBRARIES kritaui kritatestsdk
)

