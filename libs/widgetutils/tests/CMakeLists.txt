set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/libs/widgetutils )

########### next target ###############

set(kopropertiestest_SRCS KoPropertiesTest.cpp )
kde4_add_unit_test(KoPropertiesTest TESTNAME libs-widgetutils-KoPropertiesTest ${kopropertiestest_SRCS})
target_link_libraries(KoPropertiesTest  kritawidgetutils Qt5::Test)

########### next target ###############

set(kis_simple_math_parser_test_SRCS kis_simple_math_parser_test.cpp)
kde4_add_unit_test(KisSimpleMathParserTest TESTNAME krita-ui-KisSimpleMathParserTest ${kis_simple_math_parser_test_SRCS})
target_link_libraries(KisSimpleMathParserTest kritaui Qt5::Test)
