set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(     ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(kis_quickop_test_SRCS kis_quickop_test.cpp ../../../../sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(KisQuickOpTest TESTNAME krita-plugins-KisQuickOpTest ${kis_quickop_test_SRCS})
target_link_libraries(KisQuickOpTest  ${KDE4_KDEUI_LIBS} kritaquickpaintop kritaimage kritaui kritalibpaintop ${QT_QTTEST_LIBRARY})
