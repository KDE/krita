add_subdirectory( tests )

# Chose a tiles backend
# 1 - image/tiles
# 3 - image/tiles3

set(USE_TILESYSTEM 3)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-tiles.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/../config-tiles.h) ### WRONG PLACE???

if(USE_TILESYSTEM EQUAL 3)
set(libkritatile_SRCS
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile_data.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile_data_store.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile_data_pooler.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tiled_data_manager.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_memento_manager.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_hline_iterator.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_vline_iterator.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_random_accessor.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_abstract_compression.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_lzf_compression.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_abstract_tile_compressor.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_legacy_tile_compressor.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_tile_compressor_2.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_chunk_allocator.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_memory_window.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_swapped_data_store.cpp
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/swap/kis_tile_data_swapper.cpp
)
add_subdirectory( tiles3 )
endif()

option(HAVE_MEMORY_LEAK_TRACKER "Enable memory leak tracker (always disabled in release build)" OFF)
option(HAVE_BACKTRACE_SUPPORT "Enable recording of backtrace in memory leak tracker" OFF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-memory-leak-tracker.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-memory-leak-tracker.h) ### WRONG PLACE???

include_directories(
    ${CMAKE_SOURCE_DIR}/krita/image/metadata
    3rdparty
)

if(FFTW3_FOUND)
  include_directories(${FFTW3_INCLUDE_DIR})
endif()

if(HAVE_VC)
  include_directories(${Vc_INCLUDE_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})
  ko_compile_for_all_implementations(__per_arch_circle_mask_generator_objs kis_brush_mask_applicator_factories.cpp)
else()
  set(__per_arch_circle_mask_generator_objs kis_brush_mask_applicator_factories.cpp)
endif()

set(kritaimage_LIB_SRCS
   ${libkritatile_SRCS}
   kis_distance_information.cpp
   kis_painter.cc
   kis_progress_updater.cpp
   brushengine/kis_paint_information.cc
   brushengine/kis_random_source.cpp
   brushengine/kis_stroke_random_source.cpp
   brushengine/kis_paintop.cc
   brushengine/kis_paintop_factory.cpp
   brushengine/kis_paintop_preset.cpp
   brushengine/kis_paintop_registry.cc
   brushengine/kis_paintop_settings.cpp
   brushengine/kis_locked_properties.cc
   brushengine/kis_locked_properties_proxy.cpp
   brushengine/kis_locked_properties_server.cpp
   brushengine/kis_paintop_config_widget.cpp
   commands/kis_deselect_global_selection_command.cpp
   commands/kis_image_change_layers_command.cpp
   commands/kis_image_command.cpp
   commands/kis_image_set_projection_color_space_command.cpp
   commands/kis_image_layer_add_command.cpp
   commands/kis_image_layer_move_command.cpp
   commands/kis_image_layer_remove_command.cpp
   commands/kis_image_layer_remove_command_impl.cpp
   commands/kis_image_lock_command.cpp
   commands/kis_layer_command.cpp
   commands/kis_layer_props_command.cpp
   commands/kis_node_command.cpp
   commands/kis_node_compositeop_command.cpp
   commands/kis_node_opacity_command.cpp
   commands/kis_node_property_list_command.cpp
   commands/kis_reselect_global_selection_command.cpp
   commands/kis_set_global_selection_command.cpp
   commands_new/kis_saved_commands.cpp
   commands_new/kis_processing_command.cpp
   commands_new/kis_image_resize_command.cpp
   commands_new/kis_image_set_resolution_command.cpp
   commands_new/kis_node_move_command2.cpp
   commands_new/kis_set_layer_style_command.cpp
   commands_new/kis_selection_move_command2.cpp
   commands_new/kis_update_command.cpp
   commands_new/kis_switch_current_time_command.cpp
   commands_new/kis_activate_selection_mask_command.cpp
   processing/kis_do_nothing_processing_visitor.cpp
   processing/kis_simple_processing_visitor.cpp
   processing/kis_crop_processing_visitor.cpp
   processing/kis_crop_selections_processing_visitor.cpp
   processing/kis_transform_processing_visitor.cpp
   processing/kis_mirror_processing_visitor.cpp
   filter/kis_filter.cc
   filter/kis_filter_configuration.cc
   filter/kis_color_transformation_configuration.cc
   filter/kis_filter_registry.cc
   filter/kis_color_transformation_filter.cc
   generator/kis_generator.cpp
   generator/kis_generator_layer.cpp
   generator/kis_generator_registry.cpp
   floodfill/kis_fill_interval_map.cpp
   floodfill/kis_scanline_fill.cpp
   kis_adjustment_layer.cc
   kis_selection_based_layer.cpp
   kis_node_filter_interface.cpp
   kis_base_accessor.cpp
   kis_base_node.cpp
   kis_base_processor.cpp
   kis_bookmarked_configuration_manager.cc
   kis_clone_info.cpp
   kis_clone_layer.cpp
   kis_colorspace_convert_visitor.cpp
   kis_config_widget.cpp
   kis_convolution_kernel.cc
   kis_convolution_painter.cc
   kis_gaussian_kernel.cpp
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
   kis_stroke_job_strategy.cpp
   kis_stroke_strategy.cpp
   kis_stroke.cpp
   kis_strokes_queue.cpp
   kis_simple_update_queue.cpp
   kis_update_scheduler.cpp
   kis_queues_progress_updater.cpp
   kis_composite_progress_proxy.cpp
   kis_sync_lod_cache_stroke_strategy.cpp
   kis_lod_capable_layer_offset.cpp
   kis_update_time_monitor.cpp
   kis_group_layer.cc
   kis_count_visitor.cpp
   kis_histogram.cc
   kis_image_interfaces.cpp
   kis_image_animation_interface.cpp
   kis_time_range.cpp
   kis_node_graph_listener.cpp
   kis_image.cc
   kis_image_signal_router.cpp
   kis_image_config.cpp
   kis_projection_updates_filter.cpp
   kis_suspend_projection_updates_stroke_strategy.cpp
   kis_regenerate_frame_stroke_strategy.cpp
   kis_crop_saved_extra_data.cpp
   kis_signal_compressor.cpp
   kis_signal_compressor_with_param.cpp
   kis_thread_safe_signal_compressor.cpp
   kis_acyclic_signal_connector.cpp
   kis_layer.cc
   kis_indirect_painting_support.cpp
   kis_abstract_projection_plane.cpp
   kis_layer_projection_plane.cpp
   kis_command_utils.cpp
   kis_layer_utils.cpp
   kis_mask_projection_plane.cpp
   kis_projection_leaf.cpp
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
   kis_memory_leak_tracker.cpp
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
   kis_paint_layer.cc
   kis_perspective_grid.cpp
   kis_perspective_math.cpp
   kis_pixel_selection.cpp
   kis_processing_information.cpp
   kis_properties_configuration.cc
   kis_random_accessor_ng.cpp
   kis_random_generator.cc
   kis_random_sub_accessor.cpp
   kis_wrapped_random_accessor.cpp
   kis_selection.cc
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
   kis_algebra_2d.cpp
   kis_dom_utils.cpp
   kis_transparency_mask.cc
   kis_undo_store.cpp
   kis_undo_stores.cpp
   kis_undo_adapter.cpp
   kis_surrogate_undo_adapter.cpp
   kis_legacy_undo_adapter.cpp
   kis_post_execution_undo_adapter.cpp
   kis_processing_visitor.cpp
   kis_processing_applicator.cpp
   krita_utils.cpp
   kis_outline_generator.cpp
   kis_layer_composition.cpp
   kis_selection_filters.cpp
   metadata/kis_meta_data_entry.cc
   metadata/kis_meta_data_filter.cc
   metadata/kis_meta_data_filter_p.cc
   metadata/kis_meta_data_filter_registry.cc
   metadata/kis_meta_data_filter_registry_model.cc
   metadata/kis_meta_data_io_backend.cc
   metadata/kis_meta_data_merge_strategy.cc
   metadata/kis_meta_data_merge_strategy_p.cc
   metadata/kis_meta_data_merge_strategy_registry.cc
   metadata/kis_meta_data_parser.cc
   metadata/kis_meta_data_schema.cc
   metadata/kis_meta_data_schema_registry.cc
   metadata/kis_meta_data_store.cc
   metadata/kis_meta_data_type_info.cc
   metadata/kis_meta_data_validator.cc
   metadata/kis_meta_data_value.cc
   recorder/kis_action_recorder.cc
   recorder/kis_macro.cc
   recorder/kis_macro_player.cc
   recorder/kis_node_query_path.cc
   recorder/kis_play_info.cc
   recorder/kis_recorded_action.cc
   recorder/kis_recorded_action_factory_registry.cc
   recorder/kis_recorded_action_load_context.cpp
   recorder/kis_recorded_action_save_context.cpp
   recorder/kis_recorded_filter_action.cpp
   recorder/kis_recorded_fill_paint_action.cpp
   recorder/kis_recorded_node_action.cc
   recorder/kis_recorded_paint_action.cpp
   recorder/kis_recorded_path_paint_action.cpp
   recorder/kis_recorded_shape_paint_action.cpp

   kis_keyframe.cpp
   kis_keyframe_channel.cpp
   kis_scalar_keyframe_channel.cpp
   kis_raster_keyframe_channel.cpp
   kis_onion_skin_compositor.cpp
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

target_link_libraries(kritaimage kritaversion kritawidgets kritaglobal kritapsd kritaodf kritapigment kritaundo2 kritawidgetutils Qt5::Concurrent)
target_link_libraries(kritaimage LINK_INTERFACE_LIBRARIES kritaversion kritaglobal kritapsd kritaodf kritapigment kritaundo2 kritawidgetutils Qt5::Concurrent)

target_link_libraries(kritaimage ${Boost_SYSTEM_LIBRARY})

if(OPENEXR_FOUND)
  target_link_libraries(kritaimage  ${OPENEXR_LIBRARIES})
endif()

if(FFTW3_FOUND)
  target_link_libraries(kritaimage  ${FFTW3_LIBRARIES})
endif()

if(HAVE_VC)
  target_link_libraries(kritaimage  ${Vc_LIBRARIES})
endif()

if (NOT GSL_FOUND)
  message (WARNING "KRITA WARNING! No GNU Scientific Library was found! Krita's Shaped Gradients might be non-normalized! Please install GSL library.")
else ()
  target_link_libraries(kritaimage ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES})
endif ()

set_target_properties(kritaimage PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritaimage  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install schemas #############
install( FILES
    metadata/schemas/dc.schema
    metadata/schemas/exif.schema
    metadata/schemas/tiff.schema
    metadata/schemas/mkn.schema
    metadata/schemas/xmp.schema
    metadata/schemas/xmpmm.schema
    metadata/schemas/xmprights.schema

    DESTINATION ${DATA_INSTALL_DIR}/krita/metadata/schemas)
