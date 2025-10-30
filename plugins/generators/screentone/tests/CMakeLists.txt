include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-generators-")

kis_add_tests(
    KisScreentoneGeneratorTest.cpp

    LINK_LIBRARIES kritaimage kritatestsdk
)
