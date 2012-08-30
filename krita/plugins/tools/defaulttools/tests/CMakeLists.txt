set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

add_definitions(-DFILES_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
add_definitions(-DFILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}")

########### next target ###############

set(move_stroke_test_SRCS move_stroke_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(MoveStrokeTest TESTNAME krita-ui-MoveStrokeTest  ${move_stroke_test_SRCS})
target_link_libraries(MoveStrokeTest  ${KDE4_KDEUI_LIBS}  kritadefaulttools kritaui kritaimage ${QT_QTTEST_LIBRARY})
