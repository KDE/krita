########### next target ###############

set(kis_sensors_test_SRCS kis_sensors_test.cpp )
add_executable(KisSensorsTestNAME krita-paintop-SensorsTest ${kis_sensors_test_SRCS})
add_test(KisSensorsTest KisSensorsTest)
ecm_mark_as_test(KisSensorsTest)
target_link_libraries(KisSensorsTest   kritaimage kritalibpaintop Qt5::Test)

set(kis_embedded_pattern_manager_test_SRCS kis_embedded_pattern_manager_test.cpp )
add_executable(KisEmbeddedPatternManagerTestNAME krita-paintop-EmbeddedPatternManagerTest ${kis_embedded_pattern_manager_test_SRCS})
add_test(KisEmbeddedPatternManagerTest KisEmbeddedPatternManagerTest)
ecm_mark_as_test(KisEmbeddedPatternManagerTest)
target_link_libraries(KisEmbeddedPatternManagerTest   kritaimage kritalibpaintop Qt5::Test)

