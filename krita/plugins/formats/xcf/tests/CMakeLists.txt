set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############
set(kis_xcf_test_SRCS kis_xcf_test.cpp )

add_executable(kis_xcf_testNAME krita-plugin-format-xcf_test ${kis_xcf_test_SRCS})
add_test(kis_xcf_test kis_xcf_test)
ecm_mark_as_test(kis_xcf_test)

target_link_libraries(kis_xcf_test   kritaui Qt5::Test)
