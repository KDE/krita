########### next target ###############
include_directories(${CMAKE_SOURCE_DIR}/sdk/tests)

include(KritaAddBrokenUnitTest)


kis_add_tests(KisCurveOptionDataTest.cpp
    KisCurveOptionModelTest.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test)

krita_add_broken_unit_test(kis_linked_pattern_manager_test.cpp
    NAME_PREFIX "plugins-libpaintop-"
    LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test)
