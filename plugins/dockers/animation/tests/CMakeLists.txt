set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(   ${CMAKE_CURRENT_SOURCE_DIR}/..  ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

krita_add_broken_unit_test(timeline_model_test.cpp
    TEST_NAME krita-animation-TimelineModelTest
    LINK_LIBRARIES ${KDE4_KDEUI_LIBS} kritaanimationdocker kritaui kritaimage Qt5::Test)
