set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

#add_subdirectory(scratchpad)
include_directories(${CMAKE_SOURCE_DIR}/libs/image/metadata 
                    ${CMAKE_SOURCE_DIR}/sdk/tests )

include(ECMAddTests)

macro_add_unittest_definitions()

ecm_add_tests(
    kis_image_view_converter_test.cpp
    kis_shape_selection_test.cpp
    kis_doc2_test.cpp
    kis_coordinates_converter_test.cpp
    kis_grid_config_test.cpp
    kis_stabilized_events_sampler_test.cpp
    kis_derived_resources_test.cpp
    kis_brush_hud_properties_config_test.cpp
    kis_shape_commands_test.cpp
    kis_shape_layer_test.cpp
    kis_stop_gradient_editor_test.cpp
    kis_file_layer_test.cpp
    kis_multinode_property_test.cpp
    NAME_PREFIX "libs-ui-"
    LINK_LIBRARIES kritaui Qt5::Test
)

ecm_add_test( KisFrameSerializerTest.cpp
    TEST_NAME libs-ui-KisFrameSerializerTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test( KisFrameCacheStoreTest.cpp
    TEST_NAME libs-ui-KisFrameCacheStoreTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test( kis_selection_decoration_test.cpp ../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME libs-ui-KisSelectionDecorationTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test( kis_node_dummies_graph_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME libs-ui-KisNodeDummiesGraphTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test( kis_node_shapes_graph_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME libs-ui-KisNodeShapesGraphTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test( kis_model_index_converter_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME libs-ui-KisModelIndexConverterTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test( kis_categorized_list_model_test.cpp modeltest.cpp
    TEST_NAME libs-ui-KisCategorizedListModelTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test( kis_node_juggler_compressed_test.cpp  ../../../sdk/tests/testutil.cpp
    TEST_NAME libs-ui-KisNodeJugglerCompressedTest
    LINK_LIBRARIES kritaimage kritaui Qt5::Test)

ecm_add_test(
    kis_animation_exporter_test.cpp
    TEST_NAME libs-ui-animation_exporter_test
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

set(kis_node_view_test_SRCS kis_node_view_test.cpp  ../../../sdk/tests/testutil.cpp)
qt5_add_resources(kis_node_view_test_SRCS ${krita_QRCS})
ecm_add_test(${kis_node_view_test_SRCS}
    TEST_NAME libs-ui-kis_node_view_test
    LINK_LIBRARIES kritaimage kritaui Qt5::Test)



##### Tests that currently fail and should be fixed #####

include(KritaAddBrokenUnitTest)

krita_add_broken_unit_test(
    kis_node_model_test.cpp modeltest.cpp
    TEST_NAME libs-ui-kis_node_model_test
    LINK_LIBRARIES kritaui Qt5::Test)

krita_add_broken_unit_test(
    kis_shape_controller_test.cpp kis_dummies_facade_base_test.cpp
    TEST_NAME libs-ui-kis_shape_controller_test
    LINK_LIBRARIES kritaimage kritaui Qt5::Test)

krita_add_broken_unit_test(
    kis_prescaled_projection_test.cpp
    TEST_NAME libs-ui-kis_prescaled_projection_test
    LINK_LIBRARIES kritaui Qt5::Test)

krita_add_broken_unit_test(
    kis_exiv2_test.cpp
    TEST_NAME libs-ui-KisExiv2Test
    LINK_LIBRARIES kritaimage kritaui Qt5::Test)

krita_add_broken_unit_test(
    kis_clipboard_test.cpp
    TEST_NAME libs-ui-KisClipboardTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    freehand_stroke_test.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp
    TEST_NAME libs-ui-FreehandStrokeTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    FreehandStrokeBenchmark.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp
    TEST_NAME libs-ui-FreehandStrokeBenchmark
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    KisPaintOnTransparencyMaskTest.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp
    TEST_NAME libs-ui-KisPaintOnTransparencyMaskTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    fill_processing_visitor_test.cpp ${CMAKE_SOURCE_DIR}/sdk/tests/stroke_testing_utils.cpp
    TEST_NAME libs-ui-FillProcessingVisitorTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    filter_stroke_test.cpp ../../../sdk/tests/stroke_testing_utils.cpp
    TEST_NAME libs-ui-FilterStrokeTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    kis_selection_manager_test.cpp
    TEST_NAME libs-ui-KisSelectionManagerTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)
#set_tests_properties(libs-ui-KisSelectionManagerTest PROPERTIES TIMEOUT 300)

krita_add_broken_unit_test(
    kis_node_manager_test.cpp
    TEST_NAME libs-ui-KisNodeManagerTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    kis_dummies_facade_test.cpp kis_dummies_facade_base_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME libs-ui-KisDummiesFacadeTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    kis_zoom_and_pan_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME libs-ui-KisZoomAndPanTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)
#set_tests_properties(libs-ui-KisZoomAndPanTest PROPERTIES TIMEOUT 300)

krita_add_broken_unit_test(
    kis_action_manager_test.cpp
    TEST_NAME libs-ui-KisActionManagerTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    kis_categories_mapper_test.cpp testing_categories_mapper.cpp
    TEST_NAME libs-ui-KisCategoriesMapperTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    kis_asl_layer_style_serializer_test.cpp
    TEST_NAME libs-ui-KisAslLayerStyleSerializerTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    kis_animation_importer_test.cpp
    TEST_NAME libs-ui-animation_importer_test
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

krita_add_broken_unit_test(
    kis_animation_frame_cache_test.cpp
    TEST_NAME libs-ui-animation_frame_cache_test
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)

ecm_add_test(
    kis_input_manager_test.cpp ../../../sdk/tests/testutil.cpp
    TEST_NAME libs-ui-KisInputManagerTest
    LINK_LIBRARIES kritaui kritaimage Qt5::Test)
