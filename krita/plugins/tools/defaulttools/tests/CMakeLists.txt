set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(   ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(move_stroke_test_SRCS move_stroke_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(MoveStrokeTest TESTNAME krita-ui-MoveStrokeTest  ${move_stroke_test_SRCS})
target_link_libraries(MoveStrokeTest kritabasicflakes kritadefaulttools kritaui kritaimage Qt5::Test)

########### next target ###############

set(move_selection_stroke_test_SRCS move_selection_stroke_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(MoveSelectionStrokeTest TESTNAME krita-ui-MoveSelectionStrokeTest  ${move_selection_stroke_test_SRCS})
target_link_libraries(MoveSelectionStrokeTest kritabasicflakes kritadefaulttools kritaui kritaimage Qt5::Test)
