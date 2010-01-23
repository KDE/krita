set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(  ${KOMAIN_INCLUDES}  ${CMAKE_SOURCE_DIR}/krita/image/metadata ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

SET(KisRandomGeneratorDemoSources kis_random_generator_demo.cpp kimageframe.cpp)
kde4_add_ui_files(KisRandomGeneratorDemoSources kis_random_generator_demo.ui)
kde4_add_executable(KisRandomGeneratorDemo ${KisRandomGeneratorDemoSources})
target_link_libraries(KisRandomGeneratorDemo ${KDE4_KDEUI_LIBS} kritaimage ${KDEWIN_LIBRARIES})

########### next target ###############

set(kis_base_node_test_SRCS kis_base_node_test.cpp )
kde4_add_unit_test(KisBaseNodeTest TESTNAME krita-image-BaseNodeTest ${kis_base_node_test_SRCS})
target_link_libraries(KisBaseNodeTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_test_SRCS kis_node_test.cpp )
kde4_add_unit_test(KisNodeTest TESTNAME krita-image-Node-Test ${kis_node_test_SRCS})
target_link_libraries(KisNodeTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_facade_test_SRCS kis_node_facade_test.cpp )
kde4_add_unit_test(KisNodeFacadeTest TESTNAME krita-image-NodeFacadeTest ${kis_node_facade_test_SRCS})
target_link_libraries(KisNodeFacadeTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")
add_definitions(-DFILES_OUTPUT_DIR="\\"${CMAKE_CURRENT_BINARY_DIR}\\"")
set(kis_paint_device_test_SRCS kis_paint_device_test.cpp )
kde4_add_unit_test(KisPaintDeviceTest TESTNAME krita-image-KisPaintDeviceTest ${kis_paint_device_test_SRCS})
target_link_libraries(KisPaintDeviceTest  ${KDE4_KDEUI_LIBS} kritaimage kostore ${QT_QTTEST_LIBRARY})

########### next target ###############

add_definitions(-DFILES_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")
set(kis_fixed_paint_device_test_SRCS kis_fixed_paint_device_test.cpp )
kde4_add_unit_test(KisFixedPaintDeviceTest TESTNAME krita-image-KisFixedPaintDeviceTest ${kis_fixed_paint_device_test_SRCS})
target_link_libraries(KisFixedPaintDeviceTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})


########### next target ###############

set(kis_layer_test_SRCS kis_layer_test.cpp )
kde4_add_unit_test(KisLayerTest TESTNAME krita-image-KisLayerTest  ${kis_layer_test_SRCS})
target_link_libraries(KisLayerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_effect_mask_test_SRCS kis_effect_mask_test.cpp )
kde4_add_unit_test(KisEffectMaskTest TESTNAME krita-image-KisEffectMaskTest  ${kis_effect_mask_test_SRCS})
target_link_libraries(KisEffectMaskTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_iterator_test_SRCS kis_iterator_test.cpp )
kde4_add_unit_test(KisIteratorTest TESTNAME krita-image-KisIteratorTest  ${kis_iterator_test_SRCS})
target_link_libraries(KisIteratorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_painter_test_SRCS kis_painter_test.cpp )


kde4_add_unit_test(KisPainterTest TESTNAME krita-image-KisPainterTest ${kis_painter_test_SRCS})

target_link_libraries(KisPainterTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_threaded_applicator_test_SRCS kis_threaded_applicator_test.cpp )

kde4_add_unit_test(KisThreadedApplicatorTest TESTNAME krita-image-KisThreadedApplicatorTest ${kis_threaded_applicator_test_SRCS})

target_link_libraries(KisThreadedApplicatorTest  ${KDE4_KDEUI_LIBS} ${KDE4_THREADWEAVER_LIBRARY} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_selection_test_SRCS kis_selection_test.cpp )

kde4_add_unit_test(KisSelectionTest TESTNAME krita-image-SelectionTest ${kis_selection_test_SRCS})

target_link_libraries(KisSelectionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_count_visitor_test_SRCS kis_count_visitor_test.cpp )

kde4_add_unit_test(KisCountVisitorTest TESTNAME krita-image-KisCountVisitorTest ${kis_count_visitor_test_SRCS})

target_link_libraries(KisCountVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_projection_test_SRCS kis_projection_test.cpp )

kde4_add_unit_test(KisProjectionTest TESTNAME krita-image-KisProjectionTest ${kis_projection_test_SRCS})

target_link_libraries(KisProjectionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_properties_configuration_test_SRCS kis_properties_configuration_test.cpp )

kde4_add_unit_test(Kis_Properties_Configuration_Test TESTNAME krita-image-properties-configuration-Test ${kis_properties_configuration_test_SRCS})

target_link_libraries(Kis_Properties_Configuration_Test  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_transaction_test_SRCS kis_transaction_test.cpp )

kde4_add_unit_test(KisTransactionTest TESTNAME krita-image-KisTransactionTest ${kis_transaction_test_SRCS})

target_link_libraries(KisTransactionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_pixel_selection_test_SRCS kis_pixel_selection_test.cpp )

kde4_add_unit_test(KisPixelSelectionTest TESTNAME krita-image-KisPixelSelectionTest ${kis_pixel_selection_test_SRCS})

target_link_libraries(KisPixelSelectionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_group_layer_test_SRCS kis_group_layer_test.cpp )

kde4_add_unit_test(KisGroupLayerTest TESTNAME krita-image-KisGroupLayerTest ${kis_group_layer_test_SRCS})

target_link_libraries(KisGroupLayerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_mask_test_SRCS kis_filter_mask_test.cpp )

kde4_add_unit_test(KisFilterMaskTest TESTNAME krita-image-KisFilterMaskTest ${kis_filter_mask_test_SRCS})

target_link_libraries(KisFilterMaskTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_paint_layer_test_SRCS kis_paint_layer_test.cpp )

kde4_add_unit_test(KisPaintLayerTest TESTNAME krita-image-KisPaintLayerTest ${kis_paint_layer_test_SRCS})

target_link_libraries(KisPaintLayerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_action_recorder_test_SRCS kis_action_recorder_test.cpp )
kde4_add_unit_test(KisActionRecorderTest TESTNAME krita-image-KisActionRecorderTest ${kis_action_recorder_test_SRCS})
target_link_libraries(KisActionRecorderTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_adjustment_layer_test_SRCS kis_adjustment_layer_test.cpp )
kde4_add_unit_test(KisAdjustmentLayerTest TESTNAME krita-image-KisAdjustmentLayerTest ${kis_adjustment_layer_test_SRCS})
target_link_libraries(KisAdjustmentLayerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_annotation_test_SRCS kis_annotation_test.cpp )
kde4_add_unit_test(KisAnnotationTest TESTNAME krita-image-KisAnnotationTest ${kis_annotation_test_SRCS})
target_link_libraries(KisAnnotationTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_basic_math_toolbox_test_SRCS kis_basic_math_toolbox_test.cpp )
kde4_add_unit_test(KisBasicMathToolboxTest TESTNAME krita-image-KisBasicMathToolboxTest ${kis_basic_math_toolbox_test_SRCS})
target_link_libraries(KisBasicMathToolboxTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_bookmarked_configuration_manager_test_SRCS kis_bookmarked_configuration_manager_test.cpp )
kde4_add_unit_test(KisBookmarkedConfigurationManagerTest TESTNAME krita-image-KisBookmarkedConfigurationManagerTest ${kis_bookmarked_configuration_manager_test_SRCS})
target_link_libraries(KisBookmarkedConfigurationManagerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_change_profile_visitor_test_SRCS kis_change_profile_visitor_test.cpp )
kde4_add_unit_test(KisChangeProfileVisitorTest TESTNAME krita-image-KisChangeProfileVisitorTest ${kis_change_profile_visitor_test_SRCS})
target_link_libraries(KisChangeProfileVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_clone_layer_test_SRCS kis_clone_layer_test.cpp )
kde4_add_unit_test(KisCloneLayerTest TESTNAME krita-image-KisCloneLayerTest ${kis_clone_layer_test_SRCS})
target_link_libraries(KisCloneLayerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_colorspace_convert_visitor_test_SRCS kis_colorspace_convert_visitor_test.cpp )
kde4_add_unit_test(KisColorspaceConvertVisitorTest TESTNAME krita-image-KisColorspaceConvertVisitorTest ${kis_colorspace_convert_visitor_test_SRCS})
target_link_libraries(KisColorspaceConvertVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_convolution_painter_test_SRCS kis_convolution_painter_test.cpp )
kde4_add_unit_test(KisConvolutionPainterTest TESTNAME krita-image-KisConvolutionPainterTest ${kis_convolution_painter_test_SRCS})
target_link_libraries(KisConvolutionPainterTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_crop_visitor_test_SRCS kis_crop_visitor_test.cpp )
kde4_add_unit_test(KisCropVisitorTest TESTNAME krita-image-KisCropVisitorTest ${kis_crop_visitor_test_SRCS})
target_link_libraries(KisCropVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_datamanager_test_SRCS kis_datamanager_test.cpp )
kde4_add_unit_test(KisDatamanagerTest TESTNAME krita-image-KisDatamanagerTest ${kis_datamanager_test_SRCS})
target_link_libraries(KisDatamanagerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_fill_painter_test_SRCS kis_fill_painter_test.cpp )
kde4_add_unit_test(KisFillPainterTest TESTNAME krita-image-KisFillPainterTest ${kis_fill_painter_test_SRCS})
target_link_libraries(KisFillPainterTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_configuration_test_SRCS kis_filter_configuration_test.cpp )
kde4_add_unit_test(KisFilterConfigurationTest TESTNAME krita-image-KisFilterConfigurationTest ${kis_filter_configuration_test_SRCS})
target_link_libraries(KisFilterConfigurationTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_test_SRCS kis_filter_test.cpp )
kde4_add_unit_test(KisFilterTest TESTNAME krita-image-KisFilterTest ${kis_filter_test_SRCS})
target_link_libraries(KisFilterTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_selections_benchmark_SRCS kis_filter_selections_benchmark.cpp )
kde4_add_unit_test(KisFilterSelectionsBenchmark TESTNAME krita-image-KisFilterSelectionsBenchmark ${kis_filter_selections_benchmark_SRCS})
target_link_libraries(KisFilterSelectionsBenchmark  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_processing_information_test_SRCS kis_filter_processing_information_test.cpp )
kde4_add_unit_test(KisFilterProcessingInformationTest TESTNAME krita-image-KisFilterProcessingInformationTest ${kis_filter_processing_information_test_SRCS})
target_link_libraries(KisFilterProcessingInformationTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_registry_test_SRCS kis_filter_registry_test.cpp )
kde4_add_unit_test(KisFilterRegistryTest TESTNAME krita-image-KisFilterRegistryTest ${kis_filter_registry_test_SRCS})
target_link_libraries(KisFilterRegistryTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_strategy_test_SRCS kis_filter_strategy_test.cpp )
kde4_add_unit_test(KisFilterStrategyTest TESTNAME krita-image-KisFilterStrategyTest ${kis_filter_strategy_test_SRCS})
target_link_libraries(KisFilterStrategyTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_generic_colorspace_test_SRCS kis_generic_colorspace_test.cpp )
kde4_add_unit_test(KisGenericColorspaceTest TESTNAME krita-image-KisGenericColorspaceTest ${kis_generic_colorspace_test_SRCS})
target_link_libraries(KisGenericColorspaceTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_gradient_painter_test_SRCS kis_gradient_painter_test.cpp )
kde4_add_unit_test(KisGradientPainterTest TESTNAME krita-image-KisGradientPainterTest ${kis_gradient_painter_test_SRCS})
target_link_libraries(KisGradientPainterTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_histogram_test_SRCS kis_histogram_test.cpp )
kde4_add_unit_test(KisHistogramTest TESTNAME krita-image-KisHistogramTest ${kis_histogram_test_SRCS})
target_link_libraries(KisHistogramTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_image_commands_test_SRCS kis_image_commands_test.cpp )
kde4_add_unit_test(KisImageCommandsTest TESTNAME krita-image-KisImageCommandsTest ${kis_image_commands_test_SRCS})
target_link_libraries(KisImageCommandsTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_image_test_SRCS kis_image_test.cpp )
kde4_add_unit_test(KisImageTest TESTNAME krita-image-KisImageTest ${kis_image_test_SRCS})
target_link_libraries(KisImageTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_iterators_pixel_test_SRCS kis_iterators_pixel_test.cpp )
kde4_add_unit_test(KisIteratorsPixelTest TESTNAME krita-image-KisIteratorsPixelTest ${kis_iterators_pixel_test_SRCS})
target_link_libraries(KisIteratorsPixelTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_iterator_benchmark_SRCS kis_iterator_benchmark.cpp )
kde4_add_unit_test(KisIteratorBenchmark TESTNAME krita-image-KisIteratorBenchmark ${kis_iterator_benchmark_SRCS})
target_link_libraries(KisIteratorBenchmark  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_layer_commands_test_SRCS kis_layer_commands_test.cpp )
kde4_add_unit_test(KisLayerCommandsTest TESTNAME krita-image-KisLayerCommandsTest ${kis_layer_commands_test_SRCS})
target_link_libraries(KisLayerCommandsTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_macro_test_SRCS kis_macro_test.cpp )
kde4_add_unit_test(KisMacroTest TESTNAME krita-image-KisMacroTest ${kis_macro_test_SRCS})
target_link_libraries(KisMacroTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_mask_test_SRCS kis_mask_test.cpp )
kde4_add_unit_test(KisMaskTest TESTNAME krita-image-KisMaskTest ${kis_mask_test_SRCS})
target_link_libraries(KisMaskTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_math_toolbox_test_SRCS kis_math_toolbox_test.cpp )
kde4_add_unit_test(KisMathToolboxTest TESTNAME krita-image-KisMathToolboxTest ${kis_math_toolbox_test_SRCS})
target_link_libraries(KisMathToolboxTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_name_server_test_SRCS kis_name_server_test.cpp )
kde4_add_unit_test(KisNameServerTest TESTNAME krita-image-KisNameServerTest ${kis_name_server_test_SRCS})
target_link_libraries(KisNameServerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_commands_test_SRCS kis_node_commands_test.cpp )
kde4_add_unit_test(KisNodeCommandsTest TESTNAME krita-image-KisNodeCommandsTest ${kis_node_commands_test_SRCS})
target_link_libraries(KisNodeCommandsTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_graph_listener_test_SRCS kis_node_graph_listener_test.cpp )
kde4_add_unit_test(KisNodeGraphListenerTest TESTNAME krita-image-KisNodeGraphListenerTest ${kis_node_graph_listener_test_SRCS})
target_link_libraries(KisNodeGraphListenerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_node_visitor_test_SRCS kis_node_visitor_test.cpp )
kde4_add_unit_test(KisNodeVisitorTest TESTNAME krita-image-KisNodeVisitorTest ${kis_node_visitor_test_SRCS})
target_link_libraries(KisNodeVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_paint_device_action_test_SRCS kis_paint_device_action_test.cpp )
kde4_add_unit_test(KisPaintDeviceActionTest TESTNAME krita-image-KisPaintDeviceActionTest ${kis_paint_device_action_test_SRCS})
target_link_libraries(KisPaintDeviceActionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_paint_information_test_SRCS kis_paint_information_test.cpp )
kde4_add_unit_test(KisPaintInformationTest TESTNAME krita-image-KisPaintInformationTest ${kis_paint_information_test_SRCS})
target_link_libraries(KisPaintInformationTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_paintop_test_SRCS kis_paintop_test.cpp )
kde4_add_unit_test(KisPaintopTest TESTNAME krita-image-KisPaintopTest ${kis_paintop_test_SRCS})
target_link_libraries(KisPaintopTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_paintop_registry_test_SRCS kis_paintop_registry_test.cpp )
kde4_add_unit_test(KisPaintopRegistryTest TESTNAME krita-image-KisPaintopRegistryTest ${kis_paintop_registry_test_SRCS})
target_link_libraries(KisPaintopRegistryTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_paintop_settings_test_SRCS kis_paintop_settings_test.cpp )
kde4_add_unit_test(KisPaintopSettingsTest TESTNAME krita-image-KisPaintopSettingsTest ${kis_paintop_settings_test_SRCS})
target_link_libraries(KisPaintopSettingsTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_pattern_test_SRCS kis_pattern_test.cpp )
kde4_add_unit_test(KisPatternTest TESTNAME krita-image-KisPatternTest ${kis_pattern_test_SRCS})
target_link_libraries(KisPatternTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_perspective_grid_test_SRCS kis_perspective_grid_test.cpp )
kde4_add_unit_test(KisPerspectiveGridTest TESTNAME krita-image-KisPerspectiveGridTest ${kis_perspective_grid_test_SRCS})
target_link_libraries(KisPerspectiveGridTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_perspective_math_test_SRCS kis_perspective_math_test.cpp )
kde4_add_unit_test(KisPerspectiveMathTest TESTNAME krita-image-KisPerspectiveMathTest ${kis_perspective_math_test_SRCS})
target_link_libraries(KisPerspectiveMathTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_recorded_action_factory_registry_test_SRCS kis_recorded_action_factory_registry_test.cpp )
kde4_add_unit_test(KisRecordedActionFactoryRegistryTest TESTNAME krita-image-KisRecordedActionFactoryRegistryTest ${kis_recorded_action_factory_registry_test_SRCS})
target_link_libraries(KisRecordedActionFactoryRegistryTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_recorded_action_test_SRCS kis_recorded_action_test.cpp )
kde4_add_unit_test(KisRecordedActionTest TESTNAME krita-image-KisRecordedActionTest ${kis_recorded_action_test_SRCS})
target_link_libraries(KisRecordedActionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_recorded_filter_action_test_SRCS kis_recorded_filter_action_test.cpp )
kde4_add_unit_test(KisRecordedFilterActionTest TESTNAME krita-image-KisRecordedFilterActionTest ${kis_recorded_filter_action_test_SRCS})
target_link_libraries(KisRecordedFilterActionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_selected_transaction_test_SRCS kis_selected_transaction_test.cpp )
kde4_add_unit_test(KisSelectedTransactionTest TESTNAME krita-image-KisSelectedTransactionTest ${kis_selected_transaction_test_SRCS})
target_link_libraries(KisSelectedTransactionTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_selection_mask_test_SRCS kis_selection_mask_test.cpp )
kde4_add_unit_test(KisSelectionMaskTest TESTNAME krita-image-KisSelectionMaskTest ${kis_selection_mask_test_SRCS})
target_link_libraries(KisSelectionMaskTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_shared_ptr_test_SRCS kis_shared_ptr_test.cpp )
kde4_add_unit_test(KisSharedPtrTest TESTNAME krita-image-KisSharedPtrTest ${kis_shared_ptr_test_SRCS})
target_link_libraries(KisSharedPtrTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_shared_ptr_vector_test_SRCS kis_shared_ptr_vector_test.cpp )
kde4_add_unit_test(KisSharedPtrVectorTest TESTNAME krita-image-KisSharedPtrVectorTest ${kis_shared_ptr_vector_test_SRCS})
target_link_libraries(KisSharedPtrVectorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_shear_visitor_test_SRCS kis_shear_visitor_test.cpp )
kde4_add_unit_test(KisShearVisitorTest TESTNAME krita-image-KisShearVisitorTest ${kis_shear_visitor_test_SRCS})
target_link_libraries(KisShearVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_transformation_mask_test_SRCS kis_transformation_mask_test.cpp )
kde4_add_unit_test(KisTransformationMaskTest TESTNAME krita-image-KisTransformationMaskTest ${kis_transformation_mask_test_SRCS})
target_link_libraries(KisTransformationMaskTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_transform_visitor_test_SRCS kis_transform_visitor_test.cpp )
kde4_add_unit_test(KisTransformVisitorTest TESTNAME krita-image-KisTransformVisitorTest ${kis_transform_visitor_test_SRCS})
target_link_libraries(KisTransformVisitorTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_transform_worker_test_SRCS kis_transform_worker_test.cpp )
kde4_add_unit_test(KisTransformWorkerTest TESTNAME krita-image-KisTransformWorkerTest ${kis_transform_worker_test_SRCS})
target_link_libraries(KisTransformWorkerTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_transparency_mask_test_SRCS kis_transparency_mask_test.cpp )
kde4_add_unit_test(KisTransparencyMaskTest TESTNAME krita-image-KisTransparencyMaskTest ${kis_transparency_mask_test_SRCS})
target_link_libraries(KisTransparencyMaskTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_types_test_SRCS kis_types_test.cpp )
kde4_add_unit_test(KisTypesTest TESTNAME krita-image-KisTypesTest ${kis_types_test_SRCS})
target_link_libraries(KisTypesTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_vec_test_SRCS kis_vec_test.cpp )
kde4_add_unit_test(KisVecTest TESTNAME krita-image-KisVecTest ${kis_vec_test_SRCS})
target_link_libraries(KisVecTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_config_widget_test_SRCS kis_filter_config_widget_test.cpp )
kde4_add_unit_test(KisFilterConfigWidgetTest TESTNAME krita-image-KisFilterConfigWidgetTest ${kis_filter_config_widget_test_SRCS})
target_link_libraries(KisFilterConfigWidgetTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_filter_job_test_SRCS kis_filter_job_test.cpp )
kde4_add_unit_test(KisFilterJobTest TESTNAME krita-image-KisFilterJobTest ${kis_filter_job_test_SRCS})
target_link_libraries(KisFilterJobTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_meta_data_test_SRCS kis_meta_data_test.cpp )
kde4_add_unit_test(KisMetaDataTest TESTNAME krita-image-KisMetaDataTest ${kis_meta_data_test_SRCS})
target_link_libraries(KisMetaDataTest ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_random_generator_test_SRCS kis_random_generator_test.cpp )
kde4_add_unit_test(KisRandomGeneratorTest TESTNAME krita-image-KisRandomGeneratorTest ${kis_random_generator_test_SRCS})
target_link_libraries(KisRandomGeneratorTest ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_mask_generator_test_SRCS kis_mask_generator_test.cpp )
kde4_add_unit_test(KisMaskGeneratorTest TESTNAME krita-image-KisMaskGeneratorTest ${kis_mask_generator_test_SRCS})
target_link_libraries(KisMaskGeneratorTest ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

#set(kis_cs_conversion_test_SRCS kis_cs_conversion_test.cpp )
#kde4_add_unit_test(KisCsConversionTest TESTNAME krita-image-KisCsConversionTest ${kis_cs_conversion_test_SRCS})
#target_link_libraries(KisCsConversionTest ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})

########### next target ###############

set(kis_cubic_curve_test_SRCS kis_cubic_curve_test.cpp )
kde4_add_unit_test(KisCubicCurveTest TESTNAME krita-image-KisCubicCurveTest ${kis_cubic_curve_test_SRCS})
target_link_libraries(KisCubicCurveTest  ${KDE4_KDEUI_LIBS} kritaimage ${QT_QTTEST_LIBRARY})


