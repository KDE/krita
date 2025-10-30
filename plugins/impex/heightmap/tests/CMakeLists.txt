include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    kis_heightmap_test.cpp
    TEST_NAME kis_heightmap_test
    LINK_LIBRARIES kritaui kritatestsdk
    )
