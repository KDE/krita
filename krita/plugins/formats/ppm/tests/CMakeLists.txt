set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_ppm_test_SRCS kis_ppm_test.cpp )

add_executable(kis_ppm_testNAME krita-plugin-format-ppm_test ${kis_ppm_test_SRCS})
add_test(kis_ppm_test kis_ppm_test)
ecm_mark_as_test(kis_ppm_test)

target_link_libraries(kis_ppm_test   kritaui Qt5::Test)
