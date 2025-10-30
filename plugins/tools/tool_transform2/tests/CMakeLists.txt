include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-tooltransform-")

########### next target ###############

kis_add_test(test_animated_transform_parameters.cpp
    LINK_LIBRARIES kritatooltransform_static kritaui kritaimage kritatestsdk)

krita_add_broken_unit_test(TransformStrokeStrategyTest.cpp
    ../../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME TransformStrokeStrategyTest
    LINK_LIBRARIES kritatooltransform_static kritaui kritaimage kritatestsdk)
