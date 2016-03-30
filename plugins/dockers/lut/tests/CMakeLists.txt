macro_add_unittest_definitions()
include_directories( ${OCIO_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/sdk/tests)

########### next target ###############

set(kis_ocio_display_filter_test_SRCS kis_ocio_display_filter_test.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(KisOcioDisplayFilterTest TESTNAME krita-ocio-KisOcioDisplayFilterTest ${kis_ocio_display_filter_test_SRCS})
target_link_libraries(KisOcioDisplayFilterTest   kritaimage kritaui kritalutdocker ${OCIO_LIBRARIES} KF5::I18n  Qt5::Test)
