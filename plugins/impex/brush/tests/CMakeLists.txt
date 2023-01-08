macro_add_unittest_definitions()

kis_add_test(
    KisBrushTest.cpp
    TEST_NAME KisBrushTest
    LINK_LIBRARIES kritaui kritatestsdk
    NAME_PREFIX "plugins-impex-"
    )

