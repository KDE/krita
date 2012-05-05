add_subdirectory( tests )

########### next target ###############

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

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
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_rect_iterator.cpp
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
endif(USE_TILESYSTEM EQUAL 3)

option(HAVE_MEMORY_LEAK_TRACKER "Enable memory leak tracker (always disabled in release build)" OFF)
option(HAVE_BACKTRACE_SUPPORT "Enable recording of backtrace in memory leak tracker" OFF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-memory-leak-tracker.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-memory-leak-tracker.h) ### WRONG PLACE???

macro_optional_find_package(FFTW3)
macro_log_feature(FFTW3_FOUND "FFTW3" "A fast, free C FFT library" "http://www.fftw.org/" FALSE "" "Required by the Krita for fast convolution operators")
macro_bool_to_01(FFTW3_FOUND HAVE_FFTW3)
configure_file(config_convolution.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_convolution.h)

include_directories( ${KDE4_INCLUDE_DIR}/threadweaver/
                     ${CMAKE_SOURCE_DIR}/krita/image/metadata
                     ${KOMAIN_INCLUDES}
)

set(kritaimage_LIB_SRCS
   ${libkritatile_SRCS}
   kis_painter.cc
   kis_progress_updater.cpp
   brushengine/kis_paint_information.cc
   brushengine/kis_paintop.cc
   brushengine/kis_paintop_factory.cpp
   brushengine/kis_paintop_preset.cpp
   brushengine/kis_paintop_registry.cc
   brushengine/kis_paintop_settings.cpp
   commands/kis_deselect_global_selection_command.cpp
   commands/kis_image_change_layers_command.cpp
   commands/kis_image_command.cpp
   commands/kis_image_set_projection_color_space_command.cpp
   commands/kis_image_layer_add_command.cpp
   commands/kis_image_layer_move_command.cpp
   commands/kis_image_layer_remove_command.cpp
   commands/kis_image_node_lower_command.cpp
   commands/kis_image_node_raise_command.cpp
   commands/kis_image_node_to_bottom_command.cpp
   commands/kis_image_node_to_top_command.cpp
   commands/kis_image_lock_command.cpp
   commands/kis_layer_command.cpp
   commands/kis_layer_props_command.cpp
   commands/kis_node_command.cpp
   commands/kis_node_compositeop_command.cpp
   commands/kis_node_opacity_command.cpp
   commands/kis_node_move_command.cpp
   commands/kis_node_property_list_command.cpp
   commands/kis_reselect_global_selection_command.cpp
   commands/kis_set_global_selection_command.cpp
   commands_new/kis_saved_commands.cpp
   commands_new/kis_processing_command.cpp
   commands_new/kis_node_move_command2.cpp
   commands_new/kis_image_resize_command.cpp
   commands_new/kis_image_set_resolution_command.cpp
   commands_new/kis_update_command.cpp
   processing/kis_crop_processing_visitor.cpp
   processing/kis_transform_processing_visitor.cpp
   filter/kis_filter.cc
   filter/kis_filter_configuration.cc
   filter/kis_filter_job.cpp
   filter/kis_filter_registry.cc
   filter/kis_color_transformation_filter.cc
   generator/kis_generator.cpp
   generator/kis_generator_layer.cpp
   generator/kis_generator_registry.cpp
   kis_adjustment_layer.cc
   kis_selection_based_layer.cpp
   kis_background.cpp
   kis_base_accessor.cpp
   kis_base_node.cpp
   kis_base_processor.cpp
   kis_basic_math_toolbox.cpp
   kis_bookmarked_configuration_manager.cc
   kis_clone_info.cpp
   kis_clone_layer.cpp
   kis_colorspace_convert_visitor.cpp
   kis_config_widget.cpp
   kis_convolution_kernel.cc
   kis_convolution_painter.cc
   kis_cubic_curve.cpp
   kis_default_bounds.cpp
   kis_default_bounds_base.cpp
   kis_effect_mask.cc
   kis_fast_math.cpp
   kis_fill_painter.cc
   kis_filter_mask.cpp
   kis_filter_strategy.cc
   kis_gradient_painter.cc
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
   kis_group_layer.cc
   kis_count_visitor.cpp
   kis_histogram.cc
   kis_image_interfaces.cpp
   kis_node_graph_listener.cpp
   kis_image.cc
   kis_image_signal_router.cpp
   kis_image_config.cpp
   kis_layer.cc
   kis_indirect_painting_support.cpp
   kis_mask.cc
   kis_base_mask_generator.cpp
   kis_rect_mask_generator.cpp
   kis_circle_mask_generator.cpp
   kis_gauss_circle_mask_generator.cpp
   kis_gauss_rect_mask_generator.cpp
   kis_gtl_lock.cpp
   kis_curve_circle_mask_generator.cpp
   kis_curve_rect_mask_generator.cpp
   kis_math_toolbox.cpp
   kis_memory_leak_tracker.cpp
   kis_name_server.cpp
   kis_node.cpp
   kis_node_facade.cpp
   kis_node_progress_proxy.cpp
   kis_node_visitor.cpp
   kis_paint_device.cc
   kis_fixed_paint_device.cpp
   kis_paint_layer.cc
   kis_pattern.cc
   kis_perspective_grid.cpp
   kis_perspective_math.cpp
   kis_pixel_selection.cpp
   kis_processing_information.cpp
   kis_properties_configuration.cc
   kis_random_accessor_ng.cpp
   kis_random_generator.cc
   kis_random_sub_accessor.cpp
   kis_selected_transaction_data.cpp
   kis_selection.cc
   kis_selection_mask.cpp
   kis_selection_transaction_data.cpp
   kis_serializable_configuration.cc
   kis_shared.cc
   kis_threaded_applicator.cpp
   kis_transaction_data.cpp
   kis_transform_worker.cc
   kis_perspectivetransform_worker.cpp
   kis_warptransform_worker.cc
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
)


kde4_add_library(kritaimage SHARED ${kritaimage_LIB_SRCS})

target_link_libraries(kritaimage ${KDE4_KPARTS_LIBS} komain pigmentcms ${KDE4_THREADWEAVER_LIBRARIES} )
target_link_libraries(kritaimage LINK_INTERFACE_LIBRARIES pigmentcms komain ${KDE4_KPARTS_LIBS} )

if(OPENEXR_FOUND)
  target_link_libraries(kritaimage  ${OPENEXR_LIBRARIES})
endif(OPENEXR_FOUND)

if(FFTW3_FOUND)
  target_link_libraries(kritaimage  ${FFTW3_LIBRARIES})
endif(FFTW3_FOUND)


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

########### install files ###############

install( FILES 
    kis_base_node.h
    kis_base_processor.h
    kis_config_widget.h
    kis_convolution_kernel.h
    kis_convolution_painter.h
    kis_convolution_worker.h
    kis_cubic_curve.h
    kis_debug.h
    kis_default_bounds.h
    kis_distance_information.h
    filter/kis_filter.h
    filter/kis_filter_registry.h
    kis_filter_strategy.h
    kis_generic_colorspace.h
    kis_global.h
    kis_image.h
    kis_mask.h
    kis_node.h
    kis_node_facade.h
    kis_node_graph_listener.h
    kis_painter.h
    kis_paint_device.h
    kis_properties_configuration.h
    kis_processing_information.h
    kis_transform_worker.h
    kis_perspectivetransform_worker.h
    kis_warptransform_worker.h
    kis_serializable_configuration.h
    kis_selection.h
    kis_shared.h
    kis_shared_ptr.h
    kis_shared_ptr_vector.h
    kis_transaction.h
    kis_types.h
    krita_export.h
    filter/kis_filter_configuration.h
    generator/kis_generator.h
    generator/kis_generator_registry.h
    DESTINATION ${INCLUDE_INSTALL_DIR})
