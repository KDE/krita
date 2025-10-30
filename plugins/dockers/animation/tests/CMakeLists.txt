include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-dockers-animation-")

kis_add_tests(
    kis_animation_utils_test.cpp
    timeline_model_test.cpp
    LINK_LIBRARIES kritaanimationdocker_static kritaui kritaimage kritatestsdk
    )
