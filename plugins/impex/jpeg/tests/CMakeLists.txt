include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    kis_jpeg_test.cpp
    TEST_NAME kis_jpeg_test
    LINK_LIBRARIES kritametadata kritaui kritatestsdk
    )

