set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(   ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(move_stroke_test_SRCS move_stroke_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
add_executable(MoveStrokeTestNAME krita-ui-MoveStrokeTest  ${move_stroke_test_SRCS})
add_test(MoveStrokeTest MoveStrokeTest)
ecm_mark_as_test(MoveStrokeTest)
target_link_libraries(MoveStrokeTest    kritadefaulttools kritaui kritaimage Qt5::Test)
