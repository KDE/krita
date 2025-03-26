########### next target ###############


kis_add_tests(KisCurveOptionDataTest.cpp
    KisCurveOptionModelTest.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop kritatestsdk)

kis_add_tests(kis_linked_pattern_manager_test.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop kritatestsdk)
