########### next target ###############

set(kis_sensors_test_SRCS kis_sensors_test.cpp )
kde4_add_unit_test(KisSensorsTest TESTNAME krita-paintop-SensorsTest ${kis_sensors_test_SRCS})
target_link_libraries(KisSensorsTest  ${KDE4_KDEUI_LIBS} kritaimage kritalibpaintop ${QT_QTTEST_LIBRARY})

