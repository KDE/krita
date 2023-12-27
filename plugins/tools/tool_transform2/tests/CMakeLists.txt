include(KritaAddBrokenUnitTest)

########### next target ###############

kis_add_test(test_animated_transform_parameters.cpp
    NAME_PREFIX plugins-tooltransform-
    LINK_LIBRARIES kritatooltransform_static kritaui kritaimage kritatestsdk)

