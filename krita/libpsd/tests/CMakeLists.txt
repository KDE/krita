set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(kis_asl_parser_test_SRCS kis_asl_parser_test.cpp )
kde4_add_unit_test(KisAslParserTest TESTNAME kritalibpsd-asl_parser_test ${kis_asl_parser_test_SRCS})
target_link_libraries(KisAslParserTest libkispsd pigmentcms ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY})

