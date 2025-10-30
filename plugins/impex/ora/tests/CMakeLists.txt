include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    KisOraTest.cpp
    TEST_NAME KisOraTest
    LINK_LIBRARIES kritaui kritatestsdk
    )

