kis_add_tests(
    kis_animation_utils_test.cpp
    timeline_model_test.cpp
    NAME_PREFIX "plugins-dockers-animation-"
    LINK_LIBRARIES kritaanimationdocker_static kritaui kritaimage kritatestsdk
    )
