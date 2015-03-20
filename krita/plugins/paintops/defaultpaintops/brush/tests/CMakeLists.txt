set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(kis_brushop_test_SRCS kis_brushop_test.cpp ../../../../../sdk/tests/stroke_testing_utils.cpp)
add_executable(KisBrushOpTestNAME krita-plugins-KisBrushOpTest ${kis_brushop_test_SRCS})
add_test(KisBrushOpTest KisBrushOpTest)
ecm_mark_as_test(KisBrushOpTest)
target_link_libraries(KisBrushOpTest   kritaimage kritaui kritalibpaintop Qt5::Test)
