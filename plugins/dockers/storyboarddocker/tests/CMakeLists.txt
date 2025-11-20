kis_add_tests(
    StoryboardModelTest.cpp
    StoryboardTimelineSyncTest.cpp
    NAME_PREFIX "plugins-dockers-storyboarddocker-"
    LINK_LIBRARIES kritastoryboarddocker_static kritaui kritatestsdk
    )
