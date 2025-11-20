########### next target ###############

kis_add_test(
    kis_mypaintop_test.cpp ../MyPaintPaintOpSettings.cpp ../MyPaintPaintOpPreset.cpp ../MyPaintSurface.cpp
    TEST_NAME KisMyPaintOpTest
    NAME_PREFIX "plugins-kismypaintop-"
    LINK_LIBRARIES kritaimage kritamypaintop_static kritalibpaintop LibMyPaint::mypaint kritatestsdk
    )

