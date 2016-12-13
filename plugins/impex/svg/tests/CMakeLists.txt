set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

set(kis_svg_test_SRCS kis_svg_test.cpp )
kde4_add_broken_unit_test(kis_svg_test TESTNAME krita-plugins-formats-svg_test ${kis_svg_test_SRCS})
target_link_libraries(kis_svg_test   kritaui Qt5::Test)
