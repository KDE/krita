set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

set(kis_png_test_SRCS kis_png_test.cpp )
kde4_add_broken_unit_test(kis_png_test TESTNAME krita-plugins-formats-png_test ${kis_png_test_SRCS})
target_link_libraries(kis_png_test   kritaui Qt5::Test)
