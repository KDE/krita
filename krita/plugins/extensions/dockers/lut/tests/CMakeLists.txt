macro_add_unittest_definitions()
include_directories( ${OCIO_INCLUDE_DIR})

########### next target ###############

set(kis_ocio_display_filter_test_SRCS kis_ocio_display_filter_test.cpp ../../../../../sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(KisOcioDisplayFilterTest TESTNAME krita-ocio-KisOcioDisplayFilterTest ${kis_ocio_display_filter_test_SRCS})
target_link_libraries(KisOcioDisplayFilterTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui kritalutdocker ${OCIO_LIBRARIES} ${QT_QTTEST_LIBRARY})
