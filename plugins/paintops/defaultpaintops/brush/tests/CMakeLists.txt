# on macOS cannot link to a MH_LIBRARY, see bug 417391
kis_add_test(KisDabRenderingQueueTest.cpp
    TEST_NAME KisDabRenderingQueueTest
    LINK_LIBRARIES kritadefaultpaintops_static kritalibpaintop kritaimage kritatestsdk
    NAME_PREFIX "plugins-defaultpaintops-"
    )


krita_add_broken_unit_test(kis_brushop_test.cpp ../../../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME KisBrushOpTest
    LINK_LIBRARIES kritaui kritalibpaintop kritatestsdk
    NAME_PREFIX "plugins-defaultpaintops-"
    )
