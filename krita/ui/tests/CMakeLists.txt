set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KRITA_INCLUDES}   ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

add_subdirectory(scratchpad)

macro_add_unittest_definitions()

########### next target ###############

set(kis_node_model_test_SRCS kis_node_model_test.cpp modeltest.cpp )
add_executable(kis_node_model_testNAME krita-ui-kis_node_model_test  ${kis_node_model_test_SRCS})
add_test(kis_node_model_test kis_node_model_test)
ecm_mark_as_test(kis_node_model_test)
target_link_libraries(kis_node_model_test    kritaui Qt5::Test)

########### next target ###############

set(kis_image_view_converter_test_SRCS kis_image_view_converter_test.cpp )
add_executable(KisImageViewConverterTestNAME krita-ui-KisImageViewConverterTest  ${kis_image_view_converter_test_SRCS})
add_test(KisImageViewConverterTest KisImageViewConverterTest)
ecm_mark_as_test(KisImageViewConverterTest)
target_link_libraries(KisImageViewConverterTest    kritaui Qt5::Test)

########### next target ###############

set(kis_shape_controller_test_SRCS kis_shape_controller_test.cpp kis_dummies_facade_base_test.cpp)
add_executable(kis_shape_controller_testNAME krita-ui-kis_shape_controller_test  ${kis_shape_controller_test_SRCS})
add_test(kis_shape_controller_test kis_shape_controller_test)
ecm_mark_as_test(kis_shape_controller_test)
target_link_libraries(kis_shape_controller_test    kritaimage kritaui Qt5::Test)

########### next target ###############

set(squeezedcombobox_test_SRCS squeezedcombobox_test.cpp )
add_executable(squeezedcombobox_testNAME krita-ui-squeezedcombobox_test  ${squeezedcombobox_test_SRCS})
add_test(squeezedcombobox_test squeezedcombobox_test)
ecm_mark_as_test(squeezedcombobox_test)
target_link_libraries(squeezedcombobox_test   kritaui Qt5::Test)

########### next target ###############

set(kis_prescaled_projection_test_SRCS kis_prescaled_projection_test.cpp )
add_executable(KisPrescaledProjectionTestNAME krita-ui-kis_prescaled_projection_test ${kis_prescaled_projection_test_SRCS})
add_test(KisPrescaledProjectionTest KisPrescaledProjectionTest)
ecm_mark_as_test(KisPrescaledProjectionTest)
target_link_libraries(KisPrescaledProjectionTest   kritaui Qt5::Test)

########### next target ###############

set(kis_kra_loader_test_SRCS kis_kra_loader_test.cpp )
add_executable(KisKraLoaderTestNAME krita-ui-KisKraLoaderTest ${kis_kra_loader_test_SRCS})
add_test(KisKraLoaderTest KisKraLoaderTest)
ecm_mark_as_test(KisKraLoaderTest)
target_link_libraries(KisKraLoaderTest   kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_kra_saver_test_SRCS kis_kra_saver_test.cpp )
add_executable(KisKraSaverTestNAME krita-ui-KisKraSaverTest ${kis_kra_saver_test_SRCS})
add_test(KisKraSaverTest KisKraSaverTest)
ecm_mark_as_test(KisKraSaverTest)
target_link_libraries(KisKraSaverTest   kritaimage kritaui Qt5::Test)


########### next target ###############

set(kis_kra_savexml_visitor_test_SRCS kis_kra_savexml_visitor_test.cpp )
add_executable(KisKraSaveXmlVisitorTestNAME krita-ui-KisKraSaveXmlVisitorTest ${kis_kra_savexml_visitor_test_SRCS})
add_test(KisKraSaveXmlVisitorTest KisKraSaveXmlVisitorTest)
ecm_mark_as_test(KisKraSaveXmlVisitorTest)
target_link_libraries(KisKraSaveXmlVisitorTest   kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_shape_selection_test_SRCS kis_shape_selection_test.cpp )

add_executable(KisShapeSelectionTestNAME krita-ui-ShapeSelectionTest ${kis_shape_selection_test_SRCS})
add_test(KisShapeSelectionTest KisShapeSelectionTest)
ecm_mark_as_test(KisShapeSelectionTest)

target_link_libraries(KisShapeSelectionTest   kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_exiv2_test_SRCS kis_exiv2_test.cpp )
add_executable(KisExiv2TestNAME krita-ui-KisExiv2Test ${kis_exiv2_test_SRCS})
add_test(KisExiv2Test KisExiv2Test)
ecm_mark_as_test(KisExiv2Test)
target_link_libraries(KisExiv2Test   kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_recorded_action_editor_test_SRCS kis_recorded_action_editor_test.cpp )
add_executable(KisRecordedActionEditorTestNAME krita-ui-KisRecordedActionEditor ${kis_recorded_action_editor_test_SRCS})
add_test(KisRecordedActionEditorTest KisRecordedActionEditorTest)
ecm_mark_as_test(KisRecordedActionEditorTest)
target_link_libraries(KisRecordedActionEditorTest   kritaimage kritaui Qt5::Test)

########### next target ###############

set(kis_doc2_test_SRCS kis_doc2_test.cpp )
add_executable(KisDoc2TestNAME krita-ui-KisDoc2Test  ${kis_doc2_test_SRCS})
add_test(KisDoc2Test KisDoc2Test)
ecm_mark_as_test(KisDoc2Test)
target_link_libraries(KisDoc2Test    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_coordinates_converter_test_SRCS kis_coordinates_converter_test.cpp )
add_executable(KisCoordinatesConverterTestNAME krita-ui-KisCoordinatesConverterTest  ${kis_coordinates_converter_test_SRCS})
add_test(KisCoordinatesConverterTest KisCoordinatesConverterTest)
ecm_mark_as_test(KisCoordinatesConverterTest)
target_link_libraries(KisCoordinatesConverterTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_clipboard_test_SRCS kis_clipboard_test.cpp )
add_executable(KisClipboardTestNAME krita-ui-KisClipboardTest  ${kis_clipboard_test_SRCS})
add_test(KisClipboardTest KisClipboardTest)
ecm_mark_as_test(KisClipboardTest)
target_link_libraries(KisClipboardTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(freehand_stroke_test_SRCS freehand_stroke_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
add_executable(FreehandStrokeTestNAME krita-ui-FreehandStrokeTest  ${freehand_stroke_test_SRCS})
add_test(FreehandStrokeTest FreehandStrokeTest)
ecm_mark_as_test(FreehandStrokeTest)
target_link_libraries(FreehandStrokeTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(fill_processing_visitor_test_SRCS fill_processing_visitor_test.cpp ${CMAKE_SOURCE_DIR}/krita/sdk/tests/stroke_testing_utils.cpp)
add_executable(FillProcessingVisitorTestNAME krita-ui-FillProcessingVisitorTest  ${fill_processing_visitor_test_SRCS})
add_test(FillProcessingVisitorTest FillProcessingVisitorTest)
ecm_mark_as_test(FillProcessingVisitorTest)
target_link_libraries(FillProcessingVisitorTest    kritadefaulttools kritaui kritaimage Qt5::Test)


########### next target ###############

set(filter_stroke_test_SRCS filter_stroke_test.cpp ../../sdk/tests/stroke_testing_utils.cpp)
add_executable(FilterStrokeTestNAME krita-ui-FilterStrokeTest  ${filter_stroke_test_SRCS})
add_test(FilterStrokeTest FilterStrokeTest)
ecm_mark_as_test(FilterStrokeTest)
target_link_libraries(FilterStrokeTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_selection_manager_test_SRCS kis_selection_manager_test.cpp)
add_executable(KisSelectionManagerTestNAME krita-ui-KisSelectionManagerTest  ${kis_selection_manager_test_SRCS})
add_test(KisSelectionManagerTest KisSelectionManagerTest)
ecm_mark_as_test(KisSelectionManagerTest)
target_link_libraries(KisSelectionManagerTest    kritaui kritaimage Qt5::Test)
set_tests_properties(krita-ui-KisSelectionManagerTest PROPERTIES TIMEOUT 300)

########### next target ###############

set(kis_selection_decoration_test_SRCS kis_selection_decoration_test.cpp ../../sdk/tests/stroke_testing_utils.cpp)
add_executable(KisSelectionDecorationTestNAME krita-ui-KisSelectionDecorationTest  ${kis_selection_decoration_test_SRCS})
add_test(KisSelectionDecorationTest KisSelectionDecorationTest)
ecm_mark_as_test(KisSelectionDecorationTest)
target_link_libraries(KisSelectionDecorationTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_node_manager_test_SRCS kis_node_manager_test.cpp)
add_executable(KisNodeManagerTestNAME krita-ui-KisNodeManagerTest  ${kis_node_manager_test_SRCS})
add_test(KisNodeManagerTest KisNodeManagerTest)
ecm_mark_as_test(KisNodeManagerTest)
target_link_libraries(KisNodeManagerTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_node_dummies_graph_test_SRCS kis_node_dummies_graph_test.cpp ../../sdk/tests/testutil.cpp)
add_executable(KisNodeDummiesGraphTestNAME krita-ui-KisNodeDummiesGraphTest  ${kis_node_dummies_graph_test_SRCS})
add_test(KisNodeDummiesGraphTest KisNodeDummiesGraphTest)
ecm_mark_as_test(KisNodeDummiesGraphTest)
target_link_libraries(KisNodeDummiesGraphTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_node_shapes_graph_test_SRCS kis_node_shapes_graph_test.cpp ../../sdk/tests/testutil.cpp)
add_executable(KisNodeShapesGraphTestNAME krita-ui-KisNodeShapesGraphTest  ${kis_node_shapes_graph_test_SRCS})
add_test(KisNodeShapesGraphTest KisNodeShapesGraphTest)
ecm_mark_as_test(KisNodeShapesGraphTest)
target_link_libraries(KisNodeShapesGraphTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_dummies_facade_test_SRCS kis_dummies_facade_test.cpp kis_dummies_facade_base_test.cpp ../../sdk/tests/testutil.cpp)
add_executable(KisDummiesFacadeTestNAME krita-ui-KisDummiesFacadeTest  ${kis_dummies_facade_test_SRCS})
add_test(KisDummiesFacadeTest KisDummiesFacadeTest)
ecm_mark_as_test(KisDummiesFacadeTest)
target_link_libraries(KisDummiesFacadeTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_model_index_converter_test_SRCS kis_model_index_converter_test.cpp ../../sdk/tests/testutil.cpp)
add_executable(KisModelIndexConverterTestNAME krita-ui-KisModelIndexConverterTest  ${kis_model_index_converter_test_SRCS})
add_test(KisModelIndexConverterTest KisModelIndexConverterTest)
ecm_mark_as_test(KisModelIndexConverterTest)
target_link_libraries(KisModelIndexConverterTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_zoom_and_pan_test_SRCS kis_zoom_and_pan_test.cpp ../../sdk/tests/testutil.cpp)
add_executable(KisZoomAndPanTestNAME krita-ui-KisZoomAndPanTest  ${kis_zoom_and_pan_test_SRCS})
add_test(KisZoomAndPanTest KisZoomAndPanTest)
ecm_mark_as_test(KisZoomAndPanTest)
target_link_libraries(KisZoomAndPanTest    kritaui kritaimage Qt5::Test)
set_tests_properties(krita-ui-KisZoomAndPanTest PROPERTIES TIMEOUT 300)

########### next target ###############

set(kis_input_manager_test_SRCS kis_input_manager_test.cpp ../../sdk/tests/testutil.cpp)
add_executable(KisInputManagerTestNAME krita-ui-KisInputManagerTest  ${kis_input_manager_test_SRCS})
add_test(KisInputManagerTest KisInputManagerTest)
ecm_mark_as_test(KisInputManagerTest)
target_link_libraries(KisInputManagerTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_action_manager_test_SRCS kis_action_manager_test.cpp )
add_executable(KisActionManagerTestNAME krita-ui-KisActionManagerTest  ${kis_action_manager_test_SRCS})
add_test(KisActionManagerTest KisActionManagerTest)
ecm_mark_as_test(KisActionManagerTest)
target_link_libraries(KisActionManagerTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_categories_mapper_test_SRCS kis_categories_mapper_test.cpp testing_categories_mapper.cpp)
add_executable(KisCategoriesMapperTestNAME krita-ui-KisCategoriesMapperTest  ${kis_categories_mapper_test_SRCS})
add_test(KisCategoriesMapperTest KisCategoriesMapperTest)
ecm_mark_as_test(KisCategoriesMapperTest)
target_link_libraries(KisCategoriesMapperTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_categorized_list_model_test_SRCS kis_categorized_list_model_test.cpp modeltest.cpp)
add_executable(KisCategorizedListModelTestNAME krita-ui-KisCategorizedListModelTest  ${kis_categorized_list_model_test_SRCS})
add_test(KisCategorizedListModelTest KisCategorizedListModelTest)
ecm_mark_as_test(KisCategorizedListModelTest)
target_link_libraries(KisCategorizedListModelTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_resource_server_provider_test_SRCS kis_resource_server_provider_test.cpp modeltest.cpp)
add_executable(KisResourceServerProviderTestNAME krita-ui-KisResourceServerProviderTest  ${kis_resource_server_provider_test_SRCS})
add_test(KisResourceServerProviderTest KisResourceServerProviderTest)
ecm_mark_as_test(KisResourceServerProviderTest)
target_link_libraries(KisResourceServerProviderTest    kritaui kritaimage Qt5::Test)

########### next target ###############

set(kis_file_layer_test_SRCS kis_file_layer_test.cpp )
add_executable(KisFileLayerTestNAME krita-ui-KisFileLayerTest  ${kis_file_layer_test_SRCS})
add_test(KisFileLayerTest KisFileLayerTest)
ecm_mark_as_test(KisFileLayerTest)
target_link_libraries(KisFileLayerTest    kritaui kritaimage Qt5::Test)

