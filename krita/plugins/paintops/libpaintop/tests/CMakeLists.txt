########### next target ###############

set(kis_sensors_test_SRCS kis_sensors_test.cpp )
kde4_add_unit_test(KisSensorsTest TESTNAME krita-paintop-SensorsTest ${kis_sensors_test_SRCS})
target_link_libraries(KisSensorsTest  ${KDE4_KDEUI_LIBS} kritaimage kritalibpaintop ${QT_QTTEST_LIBRARY})

set(kis_embedded_pattern_manager_test_SRCS kis_embedded_pattern_manager_test.cpp )
kde4_add_unit_test(KisEmbeddedPatternManagerTest TESTNAME krita-paintop-EmbeddedPatternManagerTest ${kis_embedded_pattern_manager_test_SRCS})
target_link_libraries(KisEmbeddedPatternManagerTest  ${KDE4_KDEUI_LIBS} kritaimage kritalibpaintop ${QT_QTTEST_LIBRARY})

