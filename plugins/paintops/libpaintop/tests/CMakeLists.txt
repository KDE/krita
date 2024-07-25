########### next target ###############


kis_add_tests(KisCurveOptionDataTest.cpp
    KisCurveOptionModelTest.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop kritatestsdk)

krita_add_broken_unit_test(kis_linked_pattern_manager_test.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop kritatestsdk)
