set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KRITA_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

add_subdirectory(scratchpad)

macro_add_unittest_definitions()

########### next target ###############

set(kis_node_model_test_SRCS kis_node_model_test.cpp modeltest.cpp )
kde4_add_unit_test(kis_node_model_test TESTNAME krita-ui-kis_node_model_test  ${kis_node_model_test_SRCS})
target_link_libraries(kis_node_model_test  ${KDE4_KDEUI_LIBS}  kritaui Qt5::Test)

########### next target ###############

set(kis_image_view_converter_test_SRCS kis_image_view_converter_test.cpp )
kde4_add_unit_test(KisImageViewConverterTest TESTNAME krita-ui-KisImageViewConverterTest  ${kis_image_view_converter_test_SRCS})
target_link_libraries(KisImageViewConverterTest  ${KDE4_KDEUI_LIBS}  kritaui Qt5::Test)

########### next target ###############

set(kis_shape_controller_test_SRCS kis_shape_controller_test.cpp kis_dummies_facade_base_test.cpp)
kde4_add_unit_test(kis_shape_controller_test TESTNAME krita-ui-kis_shape_controller_test  ${kis_shape_controller_test_SRCS})
target_link_libraries(kis_shape_controller_test  ${KDE4_KDEUI_LIBS}  kritaimage kritaui Qt5::Test)

########### next target ###############

set(squeezedcombobox_test_SRCS squeezedcombobox_test.cpp )
kde4_add_unit_test(squeezedcombobox_test TESTNAME krita-ui-squeezedcombobox_test  ${squeezedcombobox_test_SRCS})
target_link_libraries(squeezedcombobox_test  ${KDE4_KDEUI_LIBS} kritaui Qt5::Test)

########### next target ###############

set(kis_prescaled_projection_test_SRCS kis_prescaled_projection_test.cpp )
kde4_add_unit_test(KisPrescaledProjectionTest TESTNAME krita-ui-kis_prescaled_projection_test ${kis_prescaled_projection_test_SRCS})
target_link_libraries(KisPrescaledProjectionTest  ${KDE4_KDEUI_LIBS} kritaui Qt5::Test)

########### next target ###############

set(kis_kra_loader_test_SRCS kis_kra_loader_test.cpp )
kde4_add_unit_test(KisKraLoaderTest TESTNAME krita-ui-KisKraLoaderTest ${kis_kra_loader_test_SRCS})
target_link_libraries(KisKraLoaderTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_kra_saver_test_SRCS kis_kra_saver_test.cpp )
kde4_add_unit_test(KisKraSaverTest TESTNAME krita-ui-KisKraSaverTest ${kis_kra_saver_test_SRCS})
target_link_libraries(KisKraSaverTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui Qt5::Test)


########### next target ###############

set(kis_kra_savexml_visitor_test_SRCS kis_kra_savexml_visitor_test.cpp )
kde4_add_unit_test(KisKraSaveXmlVisitorTest TESTNAME krita-ui-KisKraSaveXmlVisitorTest ${kis_kra_savexml_visitor_test_SRCS})
target_link_libraries(KisKraSaveXmlVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_shape_selection_test_SRCS kis_shape_selection_test.cpp )

kde4_add_unit_test(KisShapeSelectionTest TESTNAME krita-ui-ShapeSelectionTest ${kis_shape_selection_test_SRCS})

target_link_libraries(KisShapeSelectionTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_exiv2_test_SRCS kis_exiv2_test.cpp )
kde4_add_unit_test(KisExiv2Test TESTNAME krita-ui-KisExiv2Test ${kis_exiv2_test_SRCS})
target_link_libraries(KisExiv2Test  ${KDE4_KDEUI_LIBS} kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_recorded_action_editor_test_SRCS kis_recorded_action_editor_test.cpp )
kde4_add_unit_test(KisRecordedActionEditorTest TESTNAME krita-ui-KisRecordedActionEditor ${kis_recorded_action_editor_test_SRCS})
target_link_libraries(KisRecordedActionEditorTest  ${KDE4_KDEUI_LIBS} kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_doc2_test_SRCS kis_doc2_test.cpp )
kde4_add_unit_test(KisDoc2Test TESTNAME krita-ui-KisDoc2Test  ${kis_doc2_test_SRCS})
target_link_libraries(KisDoc2Test  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_coordinates_converter_test_SRCS kis_coordinates_converter_test.cpp )
kde4_add_unit_test(KisCoordinatesConverterTest TESTNAME krita-ui-KisCoordinatesConverterTest  ${kis_coordinates_converter_test_SRCS})
target_link_libraries(KisCoordinatesConverterTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_clipboard_test_SRCS kis_clipboard_test.cpp )
kde4_add_unit_test(KisClipboardTest TESTNAME krita-ui-KisClipboardTest  ${kis_clipboard_test_SRCS})
target_link_libraries(KisClipboardTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(freehand_stroke_test_SRCS freehand_stroke_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(FreehandStrokeTest TESTNAME krita-ui-FreehandStrokeTest  ${freehand_stroke_test_SRCS})
target_link_libraries(FreehandStrokeTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(fill_processing_visitor_test_SRCS fill_processing_visitor_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(FillProcessingVisitorTest TESTNAME krita-ui-FillProcessingVisitorTest  ${fill_processing_visitor_test_SRCS})
target_link_libraries(FillProcessingVisitorTest  ${KDE4_KDEUI_LIBS}  kritadefaulttools kritaui kritaimage Qt5::Test)


########### next target ###############

set(filter_stroke_test_SRCS filter_stroke_test.cpp ../../sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(FilterStrokeTest TESTNAME krita-ui-FilterStrokeTest  ${filter_stroke_test_SRCS})
target_link_libraries(FilterStrokeTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_selection_manager_test_SRCS kis_selection_manager_test.cpp)
kde4_add_unit_test(KisSelectionManagerTest TESTNAME krita-ui-KisSelectionManagerTest  ${kis_selection_manager_test_SRCS})
target_link_libraries(KisSelectionManagerTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)
set_tests_properties(krita-ui-KisSelectionManagerTest PROPERTIES TIMEOUT 300)

########### next target ###############

set(kis_selection_decoration_test_SRCS kis_selection_decoration_test.cpp ../../sdk/tests/stroke_testing_utils.cpp)
kde4_add_unit_test(KisSelectionDecorationTest TESTNAME krita-ui-KisSelectionDecorationTest  ${kis_selection_decoration_test_SRCS})
target_link_libraries(KisSelectionDecorationTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_node_manager_test_SRCS kis_node_manager_test.cpp)
kde4_add_unit_test(KisNodeManagerTest TESTNAME krita-ui-KisNodeManagerTest  ${kis_node_manager_test_SRCS})
target_link_libraries(KisNodeManagerTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_node_dummies_graph_test_SRCS kis_node_dummies_graph_test.cpp ../../sdk/tests/testutil.cpp)
kde4_add_unit_test(KisNodeDummiesGraphTest TESTNAME krita-ui-KisNodeDummiesGraphTest  ${kis_node_dummies_graph_test_SRCS})
target_link_libraries(KisNodeDummiesGraphTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_node_shapes_graph_test_SRCS kis_node_shapes_graph_test.cpp ../../sdk/tests/testutil.cpp)
kde4_add_unit_test(KisNodeShapesGraphTest TESTNAME krita-ui-KisNodeShapesGraphTest  ${kis_node_shapes_graph_test_SRCS})
target_link_libraries(KisNodeShapesGraphTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_dummies_facade_test_SRCS kis_dummies_facade_test.cpp kis_dummies_facade_base_test.cpp ../../sdk/tests/testutil.cpp)
kde4_add_unit_test(KisDummiesFacadeTest TESTNAME krita-ui-KisDummiesFacadeTest  ${kis_dummies_facade_test_SRCS})
target_link_libraries(KisDummiesFacadeTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_model_index_converter_test_SRCS kis_model_index_converter_test.cpp ../../sdk/tests/testutil.cpp)
kde4_add_unit_test(KisModelIndexConverterTest TESTNAME krita-ui-KisModelIndexConverterTest  ${kis_model_index_converter_test_SRCS})
target_link_libraries(KisModelIndexConverterTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_zoom_and_pan_test_SRCS kis_zoom_and_pan_test.cpp ../../sdk/tests/testutil.cpp)
kde4_add_unit_test(KisZoomAndPanTest TESTNAME krita-ui-KisZoomAndPanTest  ${kis_zoom_and_pan_test_SRCS})
target_link_libraries(KisZoomAndPanTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)
set_tests_properties(krita-ui-KisZoomAndPanTest PROPERTIES TIMEOUT 300)

########### next target ###############

set(kis_input_manager_test_SRCS kis_input_manager_test.cpp ../../sdk/tests/testutil.cpp)
kde4_add_unit_test(KisInputManagerTest TESTNAME krita-ui-KisInputManagerTest  ${kis_input_manager_test_SRCS})
target_link_libraries(KisInputManagerTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_action_manager_test_SRCS kis_action_manager_test.cpp )
kde4_add_unit_test(KisActionManagerTest TESTNAME krita-ui-KisActionManagerTest  ${kis_action_manager_test_SRCS})
target_link_libraries(KisActionManagerTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_categories_mapper_test_SRCS kis_categories_mapper_test.cpp testing_categories_mapper.cpp)
kde4_add_unit_test(KisCategoriesMapperTest TESTNAME krita-ui-KisCategoriesMapperTest  ${kis_categories_mapper_test_SRCS})
target_link_libraries(KisCategoriesMapperTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_categorized_list_model_test_SRCS kis_categorized_list_model_test.cpp modeltest.cpp)
kde4_add_unit_test(KisCategorizedListModelTest TESTNAME krita-ui-KisCategorizedListModelTest  ${kis_categorized_list_model_test_SRCS})
target_link_libraries(KisCategorizedListModelTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_resource_server_provider_test_SRCS kis_resource_server_provider_test.cpp modeltest.cpp)
kde4_add_unit_test(KisResourceServerProviderTest TESTNAME krita-ui-KisResourceServerProviderTest  ${kis_resource_server_provider_test_SRCS})
target_link_libraries(KisResourceServerProviderTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_file_layer_test_SRCS kis_file_layer_test.cpp )
kde4_add_unit_test(KisFileLayerTest TESTNAME krita-ui-KisFileLayerTest  ${kis_file_layer_test_SRCS})
target_link_libraries(KisFileLayerTest  ${KDE4_KDEUI_LIBS}  kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_asl_layer_style_serializer_test_SRCS kis_asl_layer_style_serializer_test.cpp )
kde4_add_unit_test(KisAslLayerStyleSerializerTest TESTNAME krita-ui-KisAslLayerStyleSerializerTest ${kis_asl_layer_style_serializer_test_SRCS})
target_link_libraries(KisAslLayerStyleSerializerTest  ${KDE4_KDEUI_LIBS} kritaui kritaimage kowidgets ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_animation_exporter_test_SRCS kis_animation_exporter_test.cpp )
kde4_add_unit_test(KisAnimationExporterTest TESTNAME kritaui-animation_exporter_test ${kis_animation_exporter_test_SRCS})
target_link_libraries(KisAnimationExporterTest  ${KDE4_KDEUI_LIBS} kritaui kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_animation_frame_cache_test_SRCS kis_animation_frame_cache_test.cpp )
kde4_add_unit_test(KisAnimationFrameCacheTest TESTNAME kritaui-animation_frame_cache_test ${kis_animation_frame_cache_test_SRCS})
target_link_libraries(KisAnimationFrameCacheTest  ${KDE4_KDEUI_LIBS} kritaui kritaimage ${QT_QTTEST_LIBRARY})
