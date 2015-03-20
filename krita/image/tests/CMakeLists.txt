set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories(    ${CMAKE_SOURCE_DIR}/krita/image/metadata ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

set(KisRandomGeneratorDemoSources kis_random_generator_demo.cpp kimageframe.cpp)
ki18n_wrap_ui(KisRandomGeneratorDemoSources kis_random_generator_demo.ui)
add_executable(KisRandomGeneratorDemo ${KisRandomGeneratorDemoSources})
target_link_libraries(KisRandomGeneratorDemo  kritaimage ${KDEWIN_LIBRARIES})

########### next target ###############

set(kis_base_node_test_SRCS kis_base_node_test.cpp )
add_executable(KisBaseNodeTestNAME krita-image-BaseNodeTest ${kis_base_node_test_SRCS})
add_test(KisBaseNodeTest KisBaseNodeTest)
ecm_mark_as_test(KisBaseNodeTest)
target_link_libraries(KisBaseNodeTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_fast_math_test_SRCS kis_fast_math_test.cpp )
add_executable(KisFastMathTestNAME krita-image-FastMathTest ${kis_fast_math_test_SRCS})
add_test(KisFastMathTest KisFastMathTest)
ecm_mark_as_test(KisFastMathTest)
target_link_libraries(KisFastMathTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_node_test_SRCS kis_node_test.cpp )
add_executable(KisNodeTestNAME krita-image-Node-Test ${kis_node_test_SRCS})
add_test(KisNodeTest KisNodeTest)
ecm_mark_as_test(KisNodeTest)
target_link_libraries(KisNodeTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_node_facade_test_SRCS kis_node_facade_test.cpp )
add_executable(KisNodeFacadeTestNAME krita-image-NodeFacadeTest ${kis_node_facade_test_SRCS})
add_test(KisNodeFacadeTest KisNodeFacadeTest)
ecm_mark_as_test(KisNodeFacadeTest)
target_link_libraries(KisNodeFacadeTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_paint_device_test_SRCS kis_paint_device_test.cpp )
add_executable(KisPaintDeviceTestNAME krita-image-KisPaintDeviceTest ${kis_paint_device_test_SRCS})
add_test(KisPaintDeviceTest KisPaintDeviceTest)
ecm_mark_as_test(KisPaintDeviceTest)
target_link_libraries(KisPaintDeviceTest   kritaimage koodf Qt5::Test)

########### next target ###############

set(kis_fixed_paint_device_test_SRCS kis_fixed_paint_device_test.cpp )
add_executable(KisFixedPaintDeviceTestNAME krita-image-KisFixedPaintDeviceTest ${kis_fixed_paint_device_test_SRCS})
add_test(KisFixedPaintDeviceTest KisFixedPaintDeviceTest)
ecm_mark_as_test(KisFixedPaintDeviceTest)
target_link_libraries(KisFixedPaintDeviceTest   kritaimage Qt5::Test)


########### next target ###############

set(kis_layer_test_SRCS kis_layer_test.cpp )
add_executable(KisLayerTestNAME krita-image-KisLayerTest  ${kis_layer_test_SRCS})
add_test(KisLayerTest KisLayerTest)
ecm_mark_as_test(KisLayerTest)
target_link_libraries(KisLayerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_effect_mask_test_SRCS kis_effect_mask_test.cpp )
add_executable(KisEffectMaskTestNAME krita-image-KisEffectMaskTest  ${kis_effect_mask_test_SRCS})
add_test(KisEffectMaskTest KisEffectMaskTest)
ecm_mark_as_test(KisEffectMaskTest)
target_link_libraries(KisEffectMaskTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_iterator_test_SRCS kis_iterator_test.cpp )
add_executable(KisIteratorTestNAME krita-image-KisIteratorTest  ${kis_iterator_test_SRCS})
add_test(KisIteratorTest KisIteratorTest)
ecm_mark_as_test(KisIteratorTest)
target_link_libraries(KisIteratorTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_painter_test_SRCS kis_painter_test.cpp )


add_executable(KisPainterTestNAME krita-image-KisPainterTest ${kis_painter_test_SRCS})
add_test(KisPainterTest KisPainterTest)
ecm_mark_as_test(KisPainterTest)

target_link_libraries(KisPainterTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_selection_test_SRCS kis_selection_test.cpp )

add_executable(KisSelectionTestNAME krita-image-SelectionTest ${kis_selection_test_SRCS})
add_test(KisSelectionTest KisSelectionTest)
ecm_mark_as_test(KisSelectionTest)

target_link_libraries(KisSelectionTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_count_visitor_test_SRCS kis_count_visitor_test.cpp )

add_executable(KisCountVisitorTestNAME krita-image-KisCountVisitorTest ${kis_count_visitor_test_SRCS})
add_test(KisCountVisitorTest KisCountVisitorTest)
ecm_mark_as_test(KisCountVisitorTest)

target_link_libraries(KisCountVisitorTest   kritaimage Qt5::Test)

########### next target ###############
set(kis_projection_test_SRCS kis_projection_test.cpp )

add_executable(KisProjectionTestNAME krita-image-KisProjectionTest ${kis_projection_test_SRCS})
add_test(KisProjectionTest KisProjectionTest)
ecm_mark_as_test(KisProjectionTest)

target_link_libraries(KisProjectionTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_properties_configuration_test_SRCS kis_properties_configuration_test.cpp )

add_executable(Kis_Properties_Configuration_TestNAME krita-image-properties-configuration-Test ${kis_properties_configuration_test_SRCS})
add_test(Kis_Properties_Configuration_Test Kis_Properties_Configuration_Test)
ecm_mark_as_test(Kis_Properties_Configuration_Test)

target_link_libraries(Kis_Properties_Configuration_Test   kritaimage Qt5::Test)

########### next target ###############

set(kis_transaction_test_SRCS kis_transaction_test.cpp )

add_executable(KisTransactionTestNAME krita-image-KisTransactionTest ${kis_transaction_test_SRCS})
add_test(KisTransactionTest KisTransactionTest)
ecm_mark_as_test(KisTransactionTest)

target_link_libraries(KisTransactionTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_pixel_selection_test_SRCS kis_pixel_selection_test.cpp )

add_executable(KisPixelSelectionTestNAME krita-image-KisPixelSelectionTest ${kis_pixel_selection_test_SRCS})
add_test(KisPixelSelectionTest KisPixelSelectionTest)
ecm_mark_as_test(KisPixelSelectionTest)

target_link_libraries(KisPixelSelectionTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_group_layer_test_SRCS kis_group_layer_test.cpp )

add_executable(KisGroupLayerTestNAME krita-image-KisGroupLayerTest ${kis_group_layer_test_SRCS})
add_test(KisGroupLayerTest KisGroupLayerTest)
ecm_mark_as_test(KisGroupLayerTest)

target_link_libraries(KisGroupLayerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_mask_test_SRCS kis_filter_mask_test.cpp )

add_executable(KisFilterMaskTestNAME krita-image-KisFilterMaskTest ${kis_filter_mask_test_SRCS})
add_test(KisFilterMaskTest KisFilterMaskTest)
ecm_mark_as_test(KisFilterMaskTest)

target_link_libraries(KisFilterMaskTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_transform_mask_test_SRCS kis_transform_mask_test.cpp )

add_executable(KisTransformMaskTestNAME krita-image-KisTransformMaskTest ${kis_transform_mask_test_SRCS})
add_test(KisTransformMaskTest KisTransformMaskTest)
ecm_mark_as_test(KisTransformMaskTest)

target_link_libraries(KisTransformMaskTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_paint_layer_test_SRCS kis_paint_layer_test.cpp )

add_executable(KisPaintLayerTestNAME krita-image-KisPaintLayerTest ${kis_paint_layer_test_SRCS})
add_test(KisPaintLayerTest KisPaintLayerTest)
ecm_mark_as_test(KisPaintLayerTest)

target_link_libraries(KisPaintLayerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_adjustment_layer_test_SRCS kis_adjustment_layer_test.cpp )
add_executable(KisAdjustmentLayerTestNAME krita-image-KisAdjustmentLayerTest ${kis_adjustment_layer_test_SRCS})
add_test(KisAdjustmentLayerTest KisAdjustmentLayerTest)
ecm_mark_as_test(KisAdjustmentLayerTest)
target_link_libraries(KisAdjustmentLayerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_annotation_test_SRCS kis_annotation_test.cpp )
add_executable(KisAnnotationTestNAME krita-image-KisAnnotationTest ${kis_annotation_test_SRCS})
add_test(KisAnnotationTest KisAnnotationTest)
ecm_mark_as_test(KisAnnotationTest)
target_link_libraries(KisAnnotationTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_basic_math_toolbox_test_SRCS kis_basic_math_toolbox_test.cpp )
add_executable(KisBasicMathToolboxTestNAME krita-image-KisBasicMathToolboxTest ${kis_basic_math_toolbox_test_SRCS})
add_test(KisBasicMathToolboxTest KisBasicMathToolboxTest)
ecm_mark_as_test(KisBasicMathToolboxTest)
target_link_libraries(KisBasicMathToolboxTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_bookmarked_configuration_manager_test_SRCS kis_bookmarked_configuration_manager_test.cpp )
add_executable(KisBookmarkedConfigurationManagerTestNAME krita-image-KisBookmarkedConfigurationManagerTest ${kis_bookmarked_configuration_manager_test_SRCS})
add_test(KisBookmarkedConfigurationManagerTest KisBookmarkedConfigurationManagerTest)
ecm_mark_as_test(KisBookmarkedConfigurationManagerTest)
target_link_libraries(KisBookmarkedConfigurationManagerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_change_profile_visitor_test_SRCS kis_change_profile_visitor_test.cpp )
add_executable(KisChangeProfileVisitorTestNAME krita-image-KisChangeProfileVisitorTest ${kis_change_profile_visitor_test_SRCS})
add_test(KisChangeProfileVisitorTest KisChangeProfileVisitorTest)
ecm_mark_as_test(KisChangeProfileVisitorTest)
target_link_libraries(KisChangeProfileVisitorTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_clone_layer_test_SRCS kis_clone_layer_test.cpp )
add_executable(KisCloneLayerTestNAME krita-image-KisCloneLayerTest ${kis_clone_layer_test_SRCS})
add_test(KisCloneLayerTest KisCloneLayerTest)
ecm_mark_as_test(KisCloneLayerTest)
target_link_libraries(KisCloneLayerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_colorspace_convert_visitor_test_SRCS kis_colorspace_convert_visitor_test.cpp )
add_executable(KisColorspaceConvertVisitorTestNAME krita-image-KisColorspaceConvertVisitorTest ${kis_colorspace_convert_visitor_test_SRCS})
add_test(KisColorspaceConvertVisitorTest KisColorspaceConvertVisitorTest)
ecm_mark_as_test(KisColorspaceConvertVisitorTest)
target_link_libraries(KisColorspaceConvertVisitorTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_convolution_painter_test_SRCS kis_convolution_painter_test.cpp )
add_executable(KisConvolutionPainterTestNAME krita-image-KisConvolutionPainterTest ${kis_convolution_painter_test_SRCS})
add_test(KisConvolutionPainterTest KisConvolutionPainterTest)
ecm_mark_as_test(KisConvolutionPainterTest)
target_link_libraries(KisConvolutionPainterTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_crop_processing_visitor_test_SRCS kis_crop_processing_visitor_test.cpp )
add_executable(KisCropProcessingVisitorTestNAME krita-image-KisCropProcessingVisitorTest ${kis_crop_processing_visitor_test_SRCS})
add_test(KisCropProcessingVisitorTest KisCropProcessingVisitorTest)
ecm_mark_as_test(KisCropProcessingVisitorTest)
target_link_libraries(KisCropProcessingVisitorTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_processing_applicator_test_SRCS kis_processing_applicator_test.cpp )
add_executable(KisProcessingApplicatorTestNAME krita-image-KisProcessingApplicatorTest ${kis_processing_applicator_test_SRCS})
add_test(KisProcessingApplicatorTest KisProcessingApplicatorTest)
ecm_mark_as_test(KisProcessingApplicatorTest)
target_link_libraries(KisProcessingApplicatorTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_datamanager_test_SRCS kis_datamanager_test.cpp )
add_executable(KisDatamanagerTestNAME krita-image-KisDatamanagerTest ${kis_datamanager_test_SRCS})
add_test(KisDatamanagerTest KisDatamanagerTest)
ecm_mark_as_test(KisDatamanagerTest)
target_link_libraries(KisDatamanagerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_fill_painter_test_SRCS kis_fill_painter_test.cpp )
add_executable(KisFillPainterTestNAME krita-image-KisFillPainterTest ${kis_fill_painter_test_SRCS})
add_test(KisFillPainterTest KisFillPainterTest)
ecm_mark_as_test(KisFillPainterTest)
target_link_libraries(KisFillPainterTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_configuration_test_SRCS kis_filter_configuration_test.cpp )
add_executable(KisFilterConfigurationTestNAME krita-image-KisFilterConfigurationTest ${kis_filter_configuration_test_SRCS})
add_test(KisFilterConfigurationTest KisFilterConfigurationTest)
ecm_mark_as_test(KisFilterConfigurationTest)
target_link_libraries(KisFilterConfigurationTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_test_SRCS kis_filter_test.cpp )
add_executable(KisFilterTestNAME krita-image-KisFilterTest ${kis_filter_test_SRCS})
add_test(KisFilterTest KisFilterTest)
ecm_mark_as_test(KisFilterTest)
target_link_libraries(KisFilterTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_processing_information_test_SRCS kis_filter_processing_information_test.cpp )
add_executable(KisFilterProcessingInformationTestNAME krita-image-KisFilterProcessingInformationTest ${kis_filter_processing_information_test_SRCS})
add_test(KisFilterProcessingInformationTest KisFilterProcessingInformationTest)
ecm_mark_as_test(KisFilterProcessingInformationTest)
target_link_libraries(KisFilterProcessingInformationTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_registry_test_SRCS kis_filter_registry_test.cpp )
add_executable(KisFilterRegistryTestNAME krita-image-KisFilterRegistryTest ${kis_filter_registry_test_SRCS})
add_test(KisFilterRegistryTest KisFilterRegistryTest)
ecm_mark_as_test(KisFilterRegistryTest)
target_link_libraries(KisFilterRegistryTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_strategy_test_SRCS kis_filter_strategy_test.cpp )
add_executable(KisFilterStrategyTestNAME krita-image-KisFilterStrategyTest ${kis_filter_strategy_test_SRCS})
add_test(KisFilterStrategyTest KisFilterStrategyTest)
ecm_mark_as_test(KisFilterStrategyTest)
target_link_libraries(KisFilterStrategyTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_gradient_painter_test_SRCS kis_gradient_painter_test.cpp )
add_executable(KisGradientPainterTestNAME krita-image-KisGradientPainterTest ${kis_gradient_painter_test_SRCS})
add_test(KisGradientPainterTest KisGradientPainterTest)
ecm_mark_as_test(KisGradientPainterTest)
target_link_libraries(KisGradientPainterTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_histogram_test_SRCS kis_histogram_test.cpp )
add_executable(KisHistogramTestNAME krita-image-KisHistogramTest ${kis_histogram_test_SRCS})
add_test(KisHistogramTest KisHistogramTest)
ecm_mark_as_test(KisHistogramTest)
target_link_libraries(KisHistogramTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_image_commands_test_SRCS kis_image_commands_test.cpp )
add_executable(KisImageCommandsTestNAME krita-image-KisImageCommandsTest ${kis_image_commands_test_SRCS})
add_test(KisImageCommandsTest KisImageCommandsTest)
ecm_mark_as_test(KisImageCommandsTest)
target_link_libraries(KisImageCommandsTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_image_test_SRCS kis_image_test.cpp )
add_executable(KisImageTestNAME krita-image-KisImageTest ${kis_image_test_SRCS})
add_test(KisImageTest KisImageTest)
ecm_mark_as_test(KisImageTest)
target_link_libraries(KisImageTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_image_signal_router_test_SRCS kis_image_signal_router_test.cpp )
add_executable(KisImageSignalRouterTestNAME krita-image-KisImageSignalRouterTest ${kis_image_signal_router_test_SRCS})
add_test(KisImageSignalRouterTest KisImageSignalRouterTest)
ecm_mark_as_test(KisImageSignalRouterTest)
target_link_libraries(KisImageSignalRouterTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_iterators_ng_test_SRCS kis_iterators_ng_test.cpp )
add_executable(KisIteratorsNGTestNAME krita-image-KisIteratorsNGTest ${kis_iterators_ng_test_SRCS})
add_test(KisIteratorsNGTest KisIteratorsNGTest)
ecm_mark_as_test(KisIteratorsNGTest)
target_link_libraries(KisIteratorsNGTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_iterator_benchmark_SRCS kis_iterator_benchmark.cpp )
add_executable(KisIteratorBenchmarkNAME krita-image-KisIteratorBenchmark ${kis_iterator_benchmark_SRCS})
add_test(KisIteratorBenchmark KisIteratorBenchmark)
ecm_mark_as_test(KisIteratorBenchmark)
target_link_libraries(KisIteratorBenchmark   kritaimage Qt5::Test)

########### next target ###############

set(kis_layer_commands_test_SRCS kis_layer_commands_test.cpp )
add_executable(KisLayerCommandsTestNAME krita-image-KisLayerCommandsTest ${kis_layer_commands_test_SRCS})
add_test(KisLayerCommandsTest KisLayerCommandsTest)
ecm_mark_as_test(KisLayerCommandsTest)
target_link_libraries(KisLayerCommandsTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_walkers_test_SRCS kis_walkers_test.cpp )
add_executable(KisWalkersTestNAME krita-image-KisWalkersTest ${kis_walkers_test_SRCS})
add_test(KisWalkersTest KisWalkersTest)
ecm_mark_as_test(KisWalkersTest)
target_link_libraries(KisWalkersTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_async_merger_test_SRCS kis_async_merger_test.cpp )
add_executable(KisAsyncMergerTestNAME krita-image-KisAsyncMergerTest ${kis_async_merger_test_SRCS})
add_test(KisAsyncMergerTest KisAsyncMergerTest)
ecm_mark_as_test(KisAsyncMergerTest)
target_link_libraries(KisAsyncMergerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_updater_context_test_SRCS kis_updater_context_test.cpp )
add_executable(KisUpdaterContextTestNAME krita-image-KisUpdaterContextTest ${kis_updater_context_test_SRCS})
add_test(KisUpdaterContextTest KisUpdaterContextTest)
ecm_mark_as_test(KisUpdaterContextTest)
target_link_libraries(KisUpdaterContextTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_simple_update_queue_test_SRCS kis_simple_update_queue_test.cpp )
add_executable(KisSimpleUpdateQueueTestNAME krita-image-KisSimpleUpdateQueueTest ${kis_simple_update_queue_test_SRCS})
add_test(KisSimpleUpdateQueueTest KisSimpleUpdateQueueTest)
ecm_mark_as_test(KisSimpleUpdateQueueTest)
target_link_libraries(KisSimpleUpdateQueueTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_update_scheduler_test_SRCS kis_update_scheduler_test.cpp )
add_executable(KisUpdateSchedulerTestNAME krita-image-KisUpdateSchedulerTest ${kis_update_scheduler_test_SRCS})
add_test(KisUpdateSchedulerTest KisUpdateSchedulerTest)
ecm_mark_as_test(KisUpdateSchedulerTest)
target_link_libraries(KisUpdateSchedulerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_stroke_test_SRCS kis_stroke_test.cpp )
add_executable(KisStrokeTestNAME krita-image-KisStrokeTest ${kis_stroke_test_SRCS})
add_test(KisStrokeTest KisStrokeTest)
ecm_mark_as_test(KisStrokeTest)
target_link_libraries(KisStrokeTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_simple_stroke_strategy_test_SRCS kis_simple_stroke_strategy_test.cpp )
add_executable(KisSimpleStrokeStrategyTestNAME krita-image-KisSimpleStrokeStrategyTest ${kis_simple_stroke_strategy_test_SRCS})
add_test(KisSimpleStrokeStrategyTest KisSimpleStrokeStrategyTest)
ecm_mark_as_test(KisSimpleStrokeStrategyTest)
target_link_libraries(KisSimpleStrokeStrategyTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_stroke_strategy_undo_command_based_test_SRCS kis_stroke_strategy_undo_command_based_test.cpp )
add_executable(KisStrokeStrategyUndoCommandBasedTestNAME krita-image-KisStrokeStrategyUndoCommandBasedTest ${kis_stroke_strategy_undo_command_based_test_SRCS})
add_test(KisStrokeStrategyUndoCommandBasedTest KisStrokeStrategyUndoCommandBasedTest)
ecm_mark_as_test(KisStrokeStrategyUndoCommandBasedTest)
target_link_libraries(KisStrokeStrategyUndoCommandBasedTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_strokes_queue_test_SRCS kis_strokes_queue_test.cpp )
add_executable(KisStrokesQueueTestNAME krita-image-KisStrokesQueueTest ${kis_strokes_queue_test_SRCS})
add_test(KisStrokesQueueTest KisStrokesQueueTest)
ecm_mark_as_test(KisStrokesQueueTest)
target_link_libraries(KisStrokesQueueTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_queues_progress_updater_test_SRCS kis_queues_progress_updater_test.cpp )
add_executable(KisQueuesProgressUpdaterTestNAME krita-image-KisQueuesProgressUpdaterTest ${kis_queues_progress_updater_test_SRCS})
add_test(KisQueuesProgressUpdaterTest KisQueuesProgressUpdaterTest)
ecm_mark_as_test(KisQueuesProgressUpdaterTest)
target_link_libraries(KisQueuesProgressUpdaterTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_macro_test_SRCS kis_macro_test.cpp )
add_executable(KisMacroTestNAME krita-image-KisMacroTest ${kis_macro_test_SRCS})
add_test(KisMacroTest KisMacroTest)
ecm_mark_as_test(KisMacroTest)
target_link_libraries(KisMacroTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_mask_test_SRCS kis_mask_test.cpp )
add_executable(KisMaskTestNAME krita-image-KisMaskTest ${kis_mask_test_SRCS})
add_test(KisMaskTest KisMaskTest)
ecm_mark_as_test(KisMaskTest)
target_link_libraries(KisMaskTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_math_toolbox_test_SRCS kis_math_toolbox_test.cpp )
add_executable(KisMathToolboxTestNAME krita-image-KisMathToolboxTest ${kis_math_toolbox_test_SRCS})
add_test(KisMathToolboxTest KisMathToolboxTest)
ecm_mark_as_test(KisMathToolboxTest)
target_link_libraries(KisMathToolboxTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_name_server_test_SRCS kis_name_server_test.cpp )
add_executable(KisNameServerTestNAME krita-image-KisNameServerTest ${kis_name_server_test_SRCS})
add_test(KisNameServerTest KisNameServerTest)
ecm_mark_as_test(KisNameServerTest)
target_link_libraries(KisNameServerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_node_commands_test_SRCS kis_node_commands_test.cpp )
add_executable(KisNodeCommandsTestNAME krita-image-KisNodeCommandsTest ${kis_node_commands_test_SRCS})
add_test(KisNodeCommandsTest KisNodeCommandsTest)
ecm_mark_as_test(KisNodeCommandsTest)
target_link_libraries(KisNodeCommandsTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_node_graph_listener_test_SRCS kis_node_graph_listener_test.cpp )
add_executable(KisNodeGraphListenerTestNAME krita-image-KisNodeGraphListenerTest ${kis_node_graph_listener_test_SRCS})
add_test(KisNodeGraphListenerTest KisNodeGraphListenerTest)
ecm_mark_as_test(KisNodeGraphListenerTest)
target_link_libraries(KisNodeGraphListenerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_node_visitor_test_SRCS kis_node_visitor_test.cpp )
add_executable(KisNodeVisitorTestNAME krita-image-KisNodeVisitorTest ${kis_node_visitor_test_SRCS})
add_test(KisNodeVisitorTest KisNodeVisitorTest)
ecm_mark_as_test(KisNodeVisitorTest)
target_link_libraries(KisNodeVisitorTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_paint_information_test_SRCS kis_paint_information_test.cpp )
add_executable(KisPaintInformationTestNAME krita-image-KisPaintInformationTest ${kis_paint_information_test_SRCS})
add_test(KisPaintInformationTest KisPaintInformationTest)
ecm_mark_as_test(KisPaintInformationTest)
target_link_libraries(KisPaintInformationTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_paintop_test_SRCS kis_paintop_test.cpp )
add_executable(KisPaintopTestNAME krita-image-KisPaintopTest ${kis_paintop_test_SRCS})
add_test(KisPaintopTest KisPaintopTest)
ecm_mark_as_test(KisPaintopTest)
target_link_libraries(KisPaintopTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_pattern_test_SRCS kis_pattern_test.cpp )
add_executable(KoPatternTestNAME krita-image-KoPatternTest ${kis_pattern_test_SRCS})
add_test(KoPatternTest KoPatternTest)
ecm_mark_as_test(KoPatternTest)
target_link_libraries(KoPatternTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_perspective_grid_test_SRCS kis_perspective_grid_test.cpp )
add_executable(KisPerspectiveGridTestNAME krita-image-KisPerspectiveGridTest ${kis_perspective_grid_test_SRCS})
add_test(KisPerspectiveGridTest KisPerspectiveGridTest)
ecm_mark_as_test(KisPerspectiveGridTest)
target_link_libraries(KisPerspectiveGridTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_recorded_action_factory_registry_test_SRCS kis_recorded_action_factory_registry_test.cpp )
add_executable(KisRecordedActionFactoryRegistryTestNAME krita-image-KisRecordedActionFactoryRegistryTest ${kis_recorded_action_factory_registry_test_SRCS})
add_test(KisRecordedActionFactoryRegistryTest KisRecordedActionFactoryRegistryTest)
ecm_mark_as_test(KisRecordedActionFactoryRegistryTest)
target_link_libraries(KisRecordedActionFactoryRegistryTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_recorded_action_test_SRCS kis_recorded_action_test.cpp )
add_executable(KisRecordedActionTestNAME krita-image-KisRecordedActionTest ${kis_recorded_action_test_SRCS})
add_test(KisRecordedActionTest KisRecordedActionTest)
ecm_mark_as_test(KisRecordedActionTest)
target_link_libraries(KisRecordedActionTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_recorded_filter_action_test_SRCS kis_recorded_filter_action_test.cpp )
add_executable(KisRecordedFilterActionTestNAME krita-image-KisRecordedFilterActionTest ${kis_recorded_filter_action_test_SRCS})
add_test(KisRecordedFilterActionTest KisRecordedFilterActionTest)
ecm_mark_as_test(KisRecordedFilterActionTest)
target_link_libraries(KisRecordedFilterActionTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_selection_mask_test_SRCS kis_selection_mask_test.cpp )
add_executable(KisSelectionMaskTestNAME krita-image-KisSelectionMaskTest ${kis_selection_mask_test_SRCS})
add_test(KisSelectionMaskTest KisSelectionMaskTest)
ecm_mark_as_test(KisSelectionMaskTest)
target_link_libraries(KisSelectionMaskTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_shared_ptr_test_SRCS kis_shared_ptr_test.cpp )
add_executable(KisSharedPtrTestNAME krita-image-KisSharedPtrTest ${kis_shared_ptr_test_SRCS})
add_test(KisSharedPtrTest KisSharedPtrTest)
ecm_mark_as_test(KisSharedPtrTest)
target_link_libraries(KisSharedPtrTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_transform_worker_test_SRCS kis_transform_worker_test.cpp )
add_executable(KisTransformWorkerTestNAME krita-image-KisTransformWorkerTest ${kis_transform_worker_test_SRCS})
add_test(KisTransformWorkerTest KisTransformWorkerTest)
ecm_mark_as_test(KisTransformWorkerTest)
target_link_libraries(KisTransformWorkerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_perspective_transform_worker_test_SRCS kis_perspective_transform_worker_test.cpp )
add_executable(KisPerspectiveTransformWorkerTestNAME krita-image-KisPerspectiveTransformWorkerTest ${kis_perspective_transform_worker_test_SRCS})
add_test(KisPerspectiveTransformWorkerTest KisPerspectiveTransformWorkerTest)
ecm_mark_as_test(KisPerspectiveTransformWorkerTest)
target_link_libraries(KisPerspectiveTransformWorkerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_bsplines_test_SRCS kis_bsplines_test.cpp )
add_executable(KisBSplinesTestNAME krita-image-KisBSplinesTest ${kis_bsplines_test_SRCS})
add_test(KisBSplinesTest KisBSplinesTest)
ecm_mark_as_test(KisBSplinesTest)
target_link_libraries(KisBSplinesTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_warp_transform_worker_test_SRCS  kis_warp_transform_worker_test.cpp)
add_executable(KisWarpTransformWorkerTestNAME krita-image-KisWarpTransformWorkerTest ${kis_warp_transform_worker_test_SRCS})
add_test(KisWarpTransformWorkerTest KisWarpTransformWorkerTest)
ecm_mark_as_test(KisWarpTransformWorkerTest)
target_link_libraries(KisWarpTransformWorkerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_cage_transform_worker_test_SRCS  kis_cage_transform_worker_test.cpp)
add_executable(KisCageTransformWorkerTestNAME krita-image-KisCageTransformWorkerTest ${kis_cage_transform_worker_test_SRCS})
add_test(KisCageTransformWorkerTest KisCageTransformWorkerTest)
ecm_mark_as_test(KisCageTransformWorkerTest)
target_link_libraries(KisCageTransformWorkerTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_liquify_transform_worker_test_SRCS  kis_liquify_transform_worker_test.cpp)
add_executable(KisLiquifyTransformWorkerTestNAME krita-image-KisLiquifyTransformWorkerTest ${kis_liquify_transform_worker_test_SRCS})
add_test(KisLiquifyTransformWorkerTest KisLiquifyTransformWorkerTest)
ecm_mark_as_test(KisLiquifyTransformWorkerTest)
target_link_libraries(KisLiquifyTransformWorkerTest   kritaimage Qt5::Test)


########### next target ###############

set(kis_transparency_mask_test_SRCS kis_transparency_mask_test.cpp )
add_executable(KisTransparencyMaskTestNAME krita-image-KisTransparencyMaskTest ${kis_transparency_mask_test_SRCS})
add_test(KisTransparencyMaskTest KisTransparencyMaskTest)
ecm_mark_as_test(KisTransparencyMaskTest)
target_link_libraries(KisTransparencyMaskTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_types_test_SRCS kis_types_test.cpp )
add_executable(KisTypesTestNAME krita-image-KisTypesTest ${kis_types_test_SRCS})
add_test(KisTypesTest KisTypesTest)
ecm_mark_as_test(KisTypesTest)
target_link_libraries(KisTypesTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_vec_test_SRCS kis_vec_test.cpp )
add_executable(KisVecTestNAME krita-image-KisVecTest ${kis_vec_test_SRCS})
add_test(KisVecTest KisVecTest)
ecm_mark_as_test(KisVecTest)
target_link_libraries(KisVecTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_config_widget_test_SRCS kis_filter_config_widget_test.cpp )
add_executable(KisFilterConfigWidgetTestNAME krita-image-KisFilterConfigWidgetTest ${kis_filter_config_widget_test_SRCS})
add_test(KisFilterConfigWidgetTest KisFilterConfigWidgetTest)
ecm_mark_as_test(KisFilterConfigWidgetTest)
target_link_libraries(KisFilterConfigWidgetTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_meta_data_test_SRCS kis_meta_data_test.cpp )
add_executable(KisMetaDataTestNAME krita-image-KisMetaDataTest ${kis_meta_data_test_SRCS})
add_test(KisMetaDataTest KisMetaDataTest)
ecm_mark_as_test(KisMetaDataTest)
target_link_libraries(KisMetaDataTest  kritaimage Qt5::Test)

########### next target ###############

set(kis_random_generator_test_SRCS kis_random_generator_test.cpp )
add_executable(KisRandomGeneratorTestNAME krita-image-KisRandomGeneratorTest ${kis_random_generator_test_SRCS})
add_test(KisRandomGeneratorTest KisRandomGeneratorTest)
ecm_mark_as_test(KisRandomGeneratorTest)
target_link_libraries(KisRandomGeneratorTest  kritaimage Qt5::Test)

########### next target ###############

set(kis_mask_generator_test_SRCS kis_mask_generator_test.cpp )
add_executable(KisMaskGeneratorTestNAME krita-image-KisMaskGeneratorTest ${kis_mask_generator_test_SRCS})
add_test(KisMaskGeneratorTest KisMaskGeneratorTest)
ecm_mark_as_test(KisMaskGeneratorTest)
target_link_libraries(KisMaskGeneratorTest  kritaimage Qt5::Test)

########### next target ###############

#set(kis_cs_conversion_test_SRCS kis_cs_conversion_test.cpp )
#add_executable(KisCsConversionTestNAME krita-image-KisCsConversionTest ${kis_cs_conversion_test_SRCS})
add_test(KisCsConversionTest KisCsConversionTest)
ecm_mark_as_test(KisCsConversionTest)
#target_link_libraries(KisCsConversionTest  kritaimage Qt5::Test)

########### next target ###############

set(kis_cubic_curve_test_SRCS kis_cubic_curve_test.cpp )
add_executable(KisCubicCurveTestNAME krita-image-KisCubicCurveTest ${kis_cubic_curve_test_SRCS})
add_test(KisCubicCurveTest KisCubicCurveTest)
ecm_mark_as_test(KisCubicCurveTest)
target_link_libraries(KisCubicCurveTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_node_query_path_test_SRCS kis_node_query_path_test.cpp )
add_executable(KisNodeQueryPathTestNAME krita-image-KisNodeQueryPathTest ${kis_node_query_path_test_SRCS})
add_test(KisNodeQueryPathTest KisNodeQueryPathTest)
ecm_mark_as_test(KisNodeQueryPathTest)
target_link_libraries(KisNodeQueryPathTest   kritaimage Qt5::Test)


########### next target ###############
# action recorder results vary with the version of lcms
if(LCMS2_FOUND)

set(kis_action_recorder_test_SRCS kis_action_recorder_test.cpp )
add_executable(KisActionRecorderTestNAME krita-image-KisActionRecorderTest ${kis_action_recorder_test_SRCS})
add_test(KisActionRecorderTest KisActionRecorderTest)
ecm_mark_as_test(KisActionRecorderTest)
target_link_libraries(KisActionRecorderTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_processings_test_SRCS kis_processings_test.cpp )
add_executable(KisProcessingsTestNAME krita-image-KisProcessingsTest ${kis_processings_test_SRCS})
add_test(KisProcessingsTest KisProcessingsTest)
ecm_mark_as_test(KisProcessingsTest)
target_link_libraries(KisProcessingsTest   kritaimage Qt5::Test)

endif()

########### next target ###############

set(kis_fixed_point_maths_test_SRCS kis_fixed_point_maths_test.cpp )
add_executable(KisFixedPointMathsTestNAME krita-image-KisFixedPointMathsTest ${kis_fixed_point_maths_test_SRCS})
add_test(KisFixedPointMathsTest KisFixedPointMathsTest)
ecm_mark_as_test(KisFixedPointMathsTest)
target_link_libraries(KisFixedPointMathsTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_weights_buffer_test_SRCS kis_filter_weights_buffer_test.cpp )
add_executable(KisFilterWeightsBufferTestNAME krita-image-KisFilterWeightsBufferTest ${kis_filter_weights_buffer_test_SRCS})
add_test(KisFilterWeightsBufferTest KisFilterWeightsBufferTest)
ecm_mark_as_test(KisFilterWeightsBufferTest)
target_link_libraries(KisFilterWeightsBufferTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_filter_weights_applicator_test_SRCS kis_filter_weights_applicator_test.cpp )
add_executable(KisFilterWeightsApplicatorTestNAME krita-image-KisFilterWeightsApplicatorTest ${kis_filter_weights_applicator_test_SRCS})
add_test(KisFilterWeightsApplicatorTest KisFilterWeightsApplicatorTest)
ecm_mark_as_test(KisFilterWeightsApplicatorTest)
target_link_libraries(KisFilterWeightsApplicatorTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_fill_interval_test_SRCS kis_fill_interval_test.cpp )
add_executable(KisFillIntervalTestNAME krita-image-FillInterval-Test ${kis_fill_interval_test_SRCS})
add_test(KisFillIntervalTest KisFillIntervalTest)
ecm_mark_as_test(KisFillIntervalTest)
target_link_libraries(KisFillIntervalTest   kritaimage Qt5::Test)


########### next target ###############

set(kis_fill_interval_map_test_SRCS kis_fill_interval_map_test.cpp )
add_executable(KisFillIntervalMapTestNAME krita-image-FillIntervalMap-Test ${kis_fill_interval_map_test_SRCS})
add_test(KisFillIntervalMapTest KisFillIntervalMapTest)
ecm_mark_as_test(KisFillIntervalMapTest)
target_link_libraries(KisFillIntervalMapTest   kritaimage Qt5::Test)

########### next target ###############

set(kis_scanline_fill_test_SRCS kis_scanline_fill_test.cpp )
add_executable(KisScanlineFillTestNAME krita-image-ScanlineFill-Test ${kis_scanline_fill_test_SRCS})
add_test(KisScanlineFillTest KisScanlineFillTest)
ecm_mark_as_test(KisScanlineFillTest)
target_link_libraries(KisScanlineFillTest   kritaimage Qt5::Test)


########### next target ###############

set(kis_dom_utils_test_SRCS kis_dom_utils_test.cpp )
add_executable(KisDomUtilsTestNAME krita-image-DomUtils-Test ${kis_dom_utils_test_SRCS})
add_test(KisDomUtilsTest KisDomUtilsTest)
ecm_mark_as_test(KisDomUtilsTest)
target_link_libraries(KisDomUtilsTest   kritaimage Qt5::Test)
