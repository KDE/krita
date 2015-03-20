set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_png_test_SRCS kis_png_test.cpp )

add_executable(kis_png_testNAME krita-plugins-formats-png_test ${kis_png_test_SRCS})
add_test(kis_png_test kis_png_test)
ecm_mark_as_test(kis_png_test)

target_link_libraries(kis_png_test   kritaui Qt5::Test)
