include(KritaAddBrokenUnitTest)

set(KIS_TEST_NAME_PREFIX "plugins-dockers-layerdocker-")

kis_add_tests(
    TestNodeView.cpp
    LINK_LIBRARIES kritaui kritalayerdocker_static kritatestsdk
    )
