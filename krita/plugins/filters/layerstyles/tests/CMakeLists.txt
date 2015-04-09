set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(kis_layer_styles_test_SRCS kis_layer_styles_test.cpp )
kde4_add_unit_test(KisLayerStylesTest TESTNAME krita-plugins-kis_layer_styles_test  ${kis_layer_styles_test_SRCS})
target_link_libraries(KisLayerStylesTest  ${KDE4_KDEUI_LIBS}  kritaimage kritalayerstylesfilter ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_layer_style_projection_plane_test_SRCS kis_layer_style_projection_plane_test.cpp )
kde4_add_unit_test(KisLayerStyleProjectionPlaneTest TESTNAME krita-plugins-layer_style_projection_plane_test ${kis_layer_style_projection_plane_test_SRCS})
target_link_libraries(KisLayerStyleProjectionPlaneTest  ${KDE4_KDEUI_LIBS} kritaimage kritalayerstylesfilter ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_asl_parser_test_SRCS kis_asl_parser_test.cpp )
kde4_add_unit_test(KisAslParserTest TESTNAME krita-plugins-asl_parser_test ${kis_asl_parser_test_SRCS})
target_link_libraries(KisAslParserTest  ${KDE4_KDEUI_LIBS} kritaimage kritalayerstylesfilter ${QT_QTTEST_LIBRARY})

