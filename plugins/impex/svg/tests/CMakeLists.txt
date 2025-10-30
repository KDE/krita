include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-impex-")

krita_add_broken_unit_test(
    kis_svg_test.cpp
    TEST_NAME KisSvgTest
    LINK_LIBRARIES kritaui kritatestsdk
    )
