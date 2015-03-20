set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_exr_test_SRCS kis_exr_test.cpp )

add_executable(kis_exr_testNAME krita-fileformat-kis_exr_test ${kis_exr_test_SRCS})
add_test(kis_exr_test kis_exr_test)
ecm_mark_as_test(kis_exr_test)

target_link_libraries(kis_exr_test   kritaui Qt5::Test)
