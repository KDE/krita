########### next target ###############
include_directories( ${CMAKE_SOURCE_DIR}/..    ${CMAKE_SOURCE_DIR}/sdk/tests )

include(KritaAddBrokenUnitTest)

ecm_add_test(kis_sensors_test.cpp
    TEST_NAME krita-paintop-SensorsTest
    LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test)

krita_add_broken_unit_test(kis_embedded_pattern_manager_test.cpp
    TEST_NAME krita-paintop-EmbeddedPatternManagerTest
    LINK_LIBRARIES kritaimage kritalibpaintop Qt5::Test)

