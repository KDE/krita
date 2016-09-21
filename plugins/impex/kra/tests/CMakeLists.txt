set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/sdk/tests )

macro_add_unittest_definitions()

set(kis_kra_loader_test_SRCS kis_kra_loader_test.cpp )
kde4_add_broken_unit_test(KisKraLoaderTest TESTNAME krita-ui-KisKraLoaderTest ${kis_kra_loader_test_SRCS})
target_link_libraries(KisKraLoaderTest   kritaimage kritaui Qt5::Test)

set(kis_kra_saver_test_SRCS kis_kra_saver_test.cpp )
kde4_add_broken_unit_test(KisKraSaverTest TESTNAME krita-ui-KisKraSaverTest ${kis_kra_saver_test_SRCS})
target_link_libraries(KisKraSaverTest   kritaimage kritaui Qt5::Test)

set(kis_kra_savexml_visitor_test_SRCS kis_kra_savexml_visitor_test.cpp ../kis_kra_savexml_visitor.cpp ../kis_kra_utils.cpp)
kde4_add_unit_test(KisKraSaveXmlVisitorTest TESTNAME krita-ui-KisKraSaveXmlVisitorTest ${kis_kra_savexml_visitor_test_SRCS})
target_link_libraries(KisKraSaveXmlVisitorTest   kritaimage kritaui Qt5::Test)

