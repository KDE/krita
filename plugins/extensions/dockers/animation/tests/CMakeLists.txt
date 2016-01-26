set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(   ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(timeline_model_test_SRCS timeline_model_test.cpp)
kde4_add_unit_test(TimelineModelTest TESTNAME krita-ui-TimelineModelTest  ${timeline_model_test_SRCS})
target_link_libraries(TimelineModelTest ${KDE4_KDEUI_LIBS} kritaanimationdocker kritaui kritaimage ${QT_QTTEST_LIBRARY})
