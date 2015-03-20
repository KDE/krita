macro_add_unittest_definitions()

########### next target ###############

set(kis_ocio_display_filter_test_SRCS kis_ocio_display_filter_test.cpp ../../../../../sdk/tests/stroke_testing_utils.cpp)
add_executable(KisOcioDisplayFilterTestNAME krita-ocio-KisOcioDisplayFilterTest ${kis_ocio_display_filter_test_SRCS})
add_test(KisOcioDisplayFilterTest KisOcioDisplayFilterTest)
ecm_mark_as_test(KisOcioDisplayFilterTest)
target_link_libraries(KisOcioDisplayFilterTest   kritaimage kritaui kritalutdocker ${OCIO_LIBRARIES} Qt5::Test)
