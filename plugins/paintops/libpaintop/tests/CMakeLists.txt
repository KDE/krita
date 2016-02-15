########### next target ###############
include_directories( ${CMAKE_SOURCE_DIR}/..    ${CMAKE_SOURCE_DIR}/sdk/tests )
set(kis_sensors_test_SRCS kis_sensors_test.cpp )
kde4_add_unit_test(KisSensorsTest TESTNAME krita-paintop-SensorsTest ${kis_sensors_test_SRCS})
target_link_libraries(KisSensorsTest   kritaimage kritalibpaintop Qt5::Test)

set(kis_embedded_pattern_manager_test_SRCS kis_embedded_pattern_manager_test.cpp )
kde4_add_unit_test(KisEmbeddedPatternManagerTest TESTNAME krita-paintop-EmbeddedPatternManagerTest ${kis_embedded_pattern_manager_test_SRCS})
target_link_libraries(KisEmbeddedPatternManagerTest   kritaimage kritalibpaintop Qt5::Test)

