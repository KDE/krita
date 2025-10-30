include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-dockers-brushud-")

kis_add_tests(
    kis_brush_hud_properties_config_test.cpp
    LINK_LIBRARIES kritaui kritabrushhud_static kritatestsdk
    )
