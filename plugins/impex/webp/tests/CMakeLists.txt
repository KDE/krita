include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    kis_webp_test.cpp
    TEST_NAME kis_webp_test
    LINK_LIBRARIES kritametadata kritaui kritatestsdk
)
