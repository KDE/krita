set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_jpeg_test_SRCS kis_jpeg_test.cpp )

add_executable(kis_jpeg_testNAME krita-plugin-format-jpeg_test ${kis_jpeg_test_SRCS})
add_test(kis_jpeg_test kis_jpeg_test)
ecm_mark_as_test(kis_jpeg_test)

target_link_libraries(kis_jpeg_test   kritaui Qt5::Test)
