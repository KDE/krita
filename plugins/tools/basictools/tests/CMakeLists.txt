set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_SOURCE_DIR}/sdk/tests
                    ${CMAKE_BINARY_DIR}/plugins/tools/basictools)



macro_add_unittest_definitions()

########### next target ###############

krita_add_broken_unit_test(move_stroke_test.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp ../strokes/move_stroke_strategy.cpp
    TEST_NAME MoveStrokeTest
    LINK_LIBRARIES kritabasicflakes kritaui Qt5::Test
    NAME_PREFIX "plugins-tools-basictools-")

########### next target ###############

ecm_add_test(move_selection_stroke_test.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp ../strokes/move_selection_stroke_strategy.cpp
    TEST_NAME MoveSelectionStrokeTest
    LINK_LIBRARIES kritabasicflakes kritaui Qt5::Test
    NAME_PREFIX "plugins-tools-basictools-")
