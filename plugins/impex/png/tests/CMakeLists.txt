include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

krita_add_broken_unit_test(
    kis_png_test.cpp
    TEST_NAME kis_png_test
    LINK_LIBRARIES kritaui kritatestsdk
)
