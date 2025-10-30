include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-dockers-storyboarddocker-")

kis_add_tests(
    StoryboardModelTest.cpp
    StoryboardTimelineSyncTest.cpp
    LINK_LIBRARIES kritastoryboarddocker_static kritaui kritatestsdk
    )
