########### next target ###############

kis_add_tests(
    kis_auto_brush_test.cpp
    kis_auto_brush_factory_test.cpp
    kis_gbr_brush_test.cpp
    kis_png_brush_test.cpp
    kis_boundary_test.cpp
    kis_imagepipe_brush_test.cpp
    TestAbrStorage.cpp
    KisBrushModelTest.cpp
    LINK_LIBRARIES kritaimage kritalibbrush kritatestsdk
    )
