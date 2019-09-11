add_subdirectory( tests )

add_subdirectory( tiles3 )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/brushengine
    ${CMAKE_CURRENT_SOURCE_DIR}/commands
    ${CMAKE_CURRENT_SOURCE_DIR}/commands_new
    ${CMAKE_CURRENT_SOURCE_DIR}/filter
    ${CMAKE_CURRENT_SOURCE_DIR}/floodfill
    ${CMAKE_CURRENT_SOURCE_DIR}/generator
    ${CMAKE_CURRENT_SOURCE_DIR}/layerstyles
    ${CMAKE_CURRENT_SOURCE_DIR}/processing

    ${CMAKE_SOURCE_DIR}/sdk/tests
)

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
)

if(FFTW3_FOUND)
  include_directories(${FFTW3_INCLUDE_DIR})
endif()

if(HAVE_VC)
  include_directories(SYSTEM ${Vc_INCLUDE_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})
  ko_compile_for_all_implementations(__per_arch_circle_mask_generator_objs kis_brush_mask_applicator_factories.cpp)
else()
  set(__per_arch_circle_mask_generator_objs kis_brush_mask_applicator_factories.cpp)
endif()

set(kritaimage_LIB_SRCS
    tiles3/kis_tile.cc
    tiles3/kis_tile_data.cc
    tiles3/kis_tile_data_store.cc
    tiles3/kis_tile_data_pooler.cc
    tiles3/kis_tiled_data_manager.cc
    tiles3/KisTiledExtentManager.cpp
    tiles3/kis_memento_manager.cc
    tiles3/kis_hline_iterator.cpp
    tiles3/kis_vline_iterator.cpp
    tiles3/kis_random_accessor.cc
    tiles3/swap/kis_abstract_compression.cpp
    tiles3/swap/kis_lzf_compression.cpp
    tiles3/swap/kis_abstract_tile_compressor.cpp
    tiles3/swap/kis_legacy_tile_compressor.cpp
    tiles3/swap/kis_tile_compressor_2.cpp
    tiles3/swap/kis_chunk_allocator.cpp
    tiles3/swap/kis_memory_window.cpp
    tiles3/swap/kis_swapped_data_store.cpp
    tiles3/swap/kis_tile_data_swapper.cpp
   kis_distance_information.cpp
   kis_painter.cc
   kis_painter_blt_multi_fixed.cpp
   kis_marker_painter.cpp
   KisPrecisePaintDeviceWrapper.cpp
   kis_progress_updater.cpp
   brushengine/kis_paint_information.cc
   brushengine/kis_random_source.cpp
   brushengine/KisPerStrokeRandomSource.cpp
   brushengine/kis_stroke_random_source.cpp
   brushengine/kis_paintop.cc
   brushengine/kis_paintop_factory.cpp
   brushengine/kis_paintop_preset.cpp
   brushengine/kis_paintop_registry.cc
   brushengine/kis_paintop_settings.cpp
   brushengine/kis_paintop_settings_update_proxy.cpp
   brushengine/kis_paintop_utils.cpp
   brushengine/kis_no_size_paintop_settings.cpp
   brushengine/kis_locked_properties.cc
   brushengine/kis_locked_properties_proxy.cpp
   brushengine/kis_locked_properties_server.cpp
   brushengine/kis_paintop_config_widget.cpp
   brushengine/kis_uniform_paintop_property.cpp
   brushengine/kis_combo_based_paintop_property.cpp
   brushengine/kis_slider_based_paintop_property.cpp
   brushengine/kis_standard_uniform_properties_factory.cpp
   brushengine/KisStrokeSpeedMeasurer.cpp
   brushengine/KisPaintopSettingsIds.cpp
   commands/kis_deselect_global_selection_command.cpp
   commands/KisDeselectActiveSelectionCommand.cpp
   commands/kis_image_change_layers_command.cpp
   commands/kis_image_change_visibility_command.cpp
   commands/kis_image_command.cpp
   commands/kis_image_set_projection_color_space_command.cpp
   commands/kis_image_layer_add_command.cpp
   commands/kis_image_layer_move_command.cpp
   commands/kis_image_layer_remove_command.cpp
   commands/kis_image_layer_remove_command_impl.cpp
   commands/kis_image_lock_command.cpp
   commands/kis_node_command.cpp
   commands/kis_node_compositeop_command.cpp
   commands/kis_node_opacity_command.cpp
   commands/kis_node_property_list_command.cpp
   commands/kis_reselect_global_selection_command.cpp
   commands/KisReselectActiveSelectionCommand.cpp
   commands/kis_set_global_selection_command.cpp
   commands/KisNodeRenameCommand.cpp
   commands_new/kis_saved_commands.cpp
   commands_new/kis_processing_command.cpp
   commands_new/kis_image_resize_command.cpp
   commands_new/kis_image_set_resolution_command.cpp
   commands_new/kis_node_move_command2.cpp
   commands_new/kis_set_layer_style_command.cpp
   commands_new/kis_selection_move_command2.cpp
   commands_new/kis_update_command.cpp
   commands_new/kis_switch_current_time_command.cpp
   commands_new/kis_change_projection_color_command.cpp
   commands_new/kis_activate_selection_mask_command.cpp
   commands_new/kis_transaction_based_command.cpp
   commands_new/KisHoldUIUpdatesCommand.cpp
   processing/kis_do_nothing_processing_visitor.cpp
   processing/kis_simple_processing_visitor.cpp
   processing/kis_convert_color_space_processing_visitor.cpp
   processing/kis_crop_processing_visitor.cpp
   processing/kis_crop_selections_processing_visitor.cpp
   processing/kis_transform_processing_visitor.cpp
   processing/kis_mirror_processing_visitor.cpp
   processing/KisSelectionBasedProcessingHelper.cpp
   filter/kis_filter.cc
   filter/kis_filter_category_ids.cpp
   filter/kis_filter_configuration.cc
   filter/kis_color_transformation_configuration.cc
   filter/kis_filter_registry.cc
   filter/kis_color_transformation_filter.cc
   generator/kis_generator.cpp
   generator/kis_generator_layer.cpp
   generator/kis_generator_registry.cpp
   floodfill/kis_fill_interval_map.cpp
   floodfill/kis_scanline_fill.cpp
   lazybrush/kis_min_cut_worker.cpp
   lazybrush/kis_lazy_fill_tools.cpp
   lazybrush/kis_multiway_cut.cpp
   lazybrush/KisWatershedWorker.cpp
   lazybrush/kis_colorize_mask.cpp
   lazybrush/kis_colorize_stroke_strategy.cpp
   KisDelayedUpdateNodeInterface.cpp
   KisCroppedOriginalLayerInterface.cpp
   kis_adjustment_layer.cc
   kis_selection_based_layer.cpp
   kis_node_filter_interface.cpp
   kis_base_accessor.cpp
   kis_base_node.cpp
   kis_base_processor.cpp
   kis_bookmarked_configuration_manager.cc
   kis_node_uuid_info.cpp
   kis_clone_layer.cpp
   kis_config_widget.cpp
   kis_convolution_kernel.cc
   kis_convolution_painter.cc
   kis_gaussian_kernel.cpp
   kis_edge_detection_kernel.cpp
   kis_cubic_curve.cpp
   kis_default_bounds.cpp
   kis_default_bounds_base.cpp
   kis_effect_mask.cc
   kis_fast_math.cpp
   kis_fill_painter.cc
   kis_filter_mask.cpp
   kis_filter_strategy.cc
   kis_transform_mask.cpp
   kis_transform_mask_params_interface.cpp
   kis_recalculate_transform_mask_job.cpp
   kis_recalculate_generator_layer_job.cpp
   kis_transform_mask_params_factory_registry.cpp
   kis_safe_transform.cpp
   kis_gradient_painter.cc
   kis_gradient_shape_strategy.cpp
   kis_cached_gradient_shape_strategy.cpp
   kis_polygonal_gradient_shape_strategy.cpp
   kis_iterator_ng.cpp
   kis_async_merger.cpp
   kis_merge_walker.cc
   kis_updater_context.cpp
   kis_update_job_item.cpp
   kis_stroke_strategy_undo_command_based.cpp
   kis_simple_stroke_strategy.cpp
   KisRunnableBasedStrokeStrategy.cpp
   KisRunnableStrokeJobDataBase.cpp
   KisRunnableStrokeJobData.cpp
   KisRunnableStrokeJobsInterface.cpp
   KisFakeRunnableStrokeJobsExecutor.cpp
   kis_stroke_job_strategy.cpp
   kis_stroke_strategy.cpp
   kis_stroke.cpp
   kis_strokes_queue.cpp
   KisStrokesQueueMutatedJobInterface.cpp
   kis_simple_update_queue.cpp
   kis_update_scheduler.cpp
   kis_queues_progress_updater.cpp
   kis_composite_progress_proxy.cpp
   kis_sync_lod_cache_stroke_strategy.cpp
   kis_lod_capable_layer_offset.cpp
   kis_update_time_monitor.cpp
   KisImageConfigNotifier.cpp
   kis_group_layer.cc
   kis_count_visitor.cpp
   kis_histogram.cc
   kis_image_interfaces.cpp
   kis_image_animation_interface.cpp
   kis_time_range.cpp
   kis_node_graph_listener.cpp
   kis_image.cc
   kis_image_signal_router.cpp
   KisImageSignals.cpp
   kis_image_config.cpp
   kis_projection_updates_filter.cpp
   kis_suspend_projection_updates_stroke_strategy.cpp
   kis_regenerate_frame_stroke_strategy.cpp
   kis_switch_time_stroke_strategy.cpp
   kis_crop_saved_extra_data.cpp
   kis_timed_signal_threshold.cpp
   kis_layer.cc
   kis_indirect_painting_support.cpp
   kis_abstract_projection_plane.cpp
   kis_layer_projection_plane.cpp
   kis_layer_utils.cpp
   kis_mask_projection_plane.cpp
   kis_projection_leaf.cpp
   KisSafeNodeProjectionStore.cpp
   kis_mask.cc
   kis_base_mask_generator.cpp
   kis_rect_mask_generator.cpp
   kis_circle_mask_generator.cpp
   kis_gauss_circle_mask_generator.cpp
   kis_gauss_rect_mask_generator.cpp
   ${__per_arch_circle_mask_generator_objs}
   kis_curve_circle_mask_generator.cpp
   kis_curve_rect_mask_generator.cpp
   kis_math_toolbox.cpp
   kis_memory_statistics_server.cpp
   kis_name_server.cpp
   kis_node.cpp
   kis_node_facade.cpp
   kis_node_progress_proxy.cpp
   kis_busy_progress_indicator.cpp
   kis_node_visitor.cpp
   kis_paint_device.cc
   kis_paint_device_debug_utils.cpp
   kis_fixed_paint_device.cpp
   KisOptimizedByteArray.cpp
   kis_paint_layer.cc
   kis_perspective_math.cpp
   kis_pixel_selection.cpp
   kis_processing_information.cpp
   kis_properties_configuration.cc
   kis_random_accessor_ng.cpp
   kis_random_generator.cc
   kis_random_sub_accessor.cpp
   kis_wrapped_random_accessor.cpp
   kis_selection.cc
   KisSelectionUpdateCompressor.cpp
   kis_selection_mask.cpp
   kis_update_outline_job.cpp
   kis_update_selection_job.cpp
   kis_serializable_configuration.cc
   kis_transaction_data.cpp
   kis_transform_worker.cc
   kis_perspectivetransform_worker.cpp
   bsplines/kis_bspline_1d.cpp
   bsplines/kis_bspline_2d.cpp
   bsplines/kis_nu_bspline_2d.cpp
   kis_warptransform_worker.cc
   kis_cage_transform_worker.cpp
   kis_liquify_transform_worker.cpp
   kis_green_coordinates_math.cpp
   kis_transparency_mask.cc
   kis_undo_adapter.cpp
   kis_macro_based_undo_store.cpp
   kis_surrogate_undo_adapter.cpp
   kis_legacy_undo_adapter.cpp
   kis_post_execution_undo_adapter.cpp
   kis_processing_visitor.cpp
   kis_processing_applicator.cpp
   krita_utils.cpp
   kis_outline_generator.cpp
   kis_layer_composition.cpp
   kis_selection_filters.cpp
   KisProofingConfiguration.h
   KisRecycleProjectionsJob.cpp

   kis_keyframe.cpp
   kis_keyframe_channel.cpp
   kis_keyframe_commands.cpp
   kis_scalar_keyframe_channel.cpp
   kis_raster_keyframe_channel.cpp
   kis_onion_skin_compositor.cpp
   kis_onion_skin_cache.cpp
   kis_idle_watcher.cpp

   kis_psd_layer_style.cpp
   kis_layer_properties_icons.cpp

   layerstyles/kis_multiple_projection.cpp
   layerstyles/kis_layer_style_filter.cpp
   layerstyles/kis_layer_style_filter_environment.cpp
   layerstyles/kis_layer_style_filter_projection_plane.cpp
   layerstyles/kis_layer_style_projection_plane.cpp
   layerstyles/kis_ls_drop_shadow_filter.cpp
   layerstyles/kis_ls_satin_filter.cpp
   layerstyles/kis_ls_stroke_filter.cpp
   layerstyles/kis_ls_bevel_emboss_filter.cpp
   layerstyles/kis_ls_overlay_filter.cpp
   layerstyles/kis_ls_utils.cpp
   layerstyles/gimp_bump_map.cpp

   KisProofingConfiguration.cpp

   kis_node_query_path.cc
)

set(einspline_SRCS
   3rdparty/einspline/bspline_create.cpp
   3rdparty/einspline/bspline_data.cpp
   3rdparty/einspline/multi_bspline_create.cpp
   3rdparty/einspline/nubasis.cpp
   3rdparty/einspline/nubspline_create.cpp
   3rdparty/einspline/nugrid.cpp
)

add_library(kritaimage SHARED ${kritaimage_LIB_SRCS} ${einspline_SRCS})
generate_export_header(kritaimage BASE_NAME kritaimage)

target_link_libraries(kritaimage 
    PUBLIC
        kritaversion 
        kritawidgets 
        kritaglobal 
        kritapsd 
        kritaodf 
        kritapigment 
        kritacommand
        kritawidgetutils 
        kritametadata
        Qt5::Concurrent
)

target_link_libraries(kritaimage PUBLIC ${Boost_SYSTEM_LIBRARY})

if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB)
    if(NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
        target_link_libraries(kritaimage PUBLIC atomic)
    endif()
endif()

if(OPENEXR_FOUND)
  target_link_libraries(kritaimage PUBLIC ${OPENEXR_LIBRARIES})
endif()

if(FFTW3_FOUND)
  target_link_libraries(kritaimage PRIVATE ${FFTW3_LIBRARIES})
endif()

if(HAVE_VC)
  target_link_libraries(kritaimage PUBLIC ${Vc_LIBRARIES})
endif()

if (NOT GSL_FOUND)
  message (WARNING "KRITA WARNING! No GNU Scientific Library was found! Krita's Shaped Gradients might be non-normalized! Please install GSL library.")
else ()
  target_link_libraries(kritaimage PRIVATE ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
endif ()

target_include_directories(kritaimage
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/brushengine>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/filter>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/generator>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/layerstyles>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/processing>
)

set_target_properties(kritaimage PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaimage  ${INSTALL_TARGETS_DEFAULT_ARGS})
