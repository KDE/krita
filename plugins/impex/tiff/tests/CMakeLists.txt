include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

kis_add_test(
    kis_tiff_test.cpp
    LINK_LIBRARIES kritaui kritatestsdk
    )

