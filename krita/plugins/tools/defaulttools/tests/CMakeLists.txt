set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(move_stroke_test_SRCS move_stroke_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(MoveStrokeTest TESTNAME krita-ui-MoveStrokeTest  ${move_stroke_test_SRCS})
target_link_libraries(MoveStrokeTest  ${KDE4_KDEUI_LIBS}  kritadefaulttools kritaui kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(fill_processing_visitor_test_SRCS fill_processing_visitor_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(FillProcessingVisitorTest TESTNAME krita-ui-FillProcessingVisitorTest  ${fill_processing_visitor_test_SRCS})
target_link_libraries(FillProcessingVisitorTest  ${KDE4_KDEUI_LIBS}  kritadefaulttools kritaui kritaimage ${QT_QTTEST_LIBRARY})
