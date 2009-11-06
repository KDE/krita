add_subdirectory( tests )

########### next target ###############

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

# Chose a tiles backend 
# 1 - image/tiles
# 3 - image/tiles3

set(USE_TILESYSTEM 3)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-tiles.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/../config-tiles.h) ### WRONG PLACE???

if(USE_TILESYSTEM EQUAL 1)
set(libkritatile_SRCS
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tiled_random_accessor.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tiledvlineiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tiledhlineiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tileddatamanager.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tile.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tilediterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tiledrectiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_memento.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tilemanager.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles/kis_tile_compressor.cc
)
add_subdirectory( tiles )
elseif(USE_TILESYSTEM EQUAL 3)
set(libkritatile_SRCS
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile_data.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile_data_store.cc
    ${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tile_data_pooler.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tiled_data_manager.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_memento_manager.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tilediterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tiledrectiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tiledvlineiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tiledhlineiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles3/kis_tiled_random_accessor.cc
)
add_subdirectory( tiles3 )
endif(USE_TILESYSTEM EQUAL 1)

include_directories( ${KDE4_INCLUDE_DIR}/threadweaver/
                     ${CMAKE_SOURCE_DIR}/krita/image/metadata
                     ${KOMAIN_INCLUDES}
                     ${KOBASE_INCLUDES}
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
   commands/kis_deselect_local_selection_command.cpp
   commands/kis_image_change_layers_command.cpp
   commands/kis_image_command.cpp
   commands/kis_image_convert_type_command.cpp
   commands/kis_image_layer_add_command.cpp
   commands/kis_image_layer_move_command.cpp
   commands/kis_image_layer_remove_command.cpp
   commands/kis_image_node_lower_command.cpp
   commands/kis_image_node_raise_command.cpp
   commands/kis_image_node_to_bottom_command.cpp
   commands/kis_image_node_to_top_command.cpp
   commands/kis_image_lock_command.cpp
   commands/kis_image_props_command.cpp
   commands/kis_image_resize_command.cpp
   commands/kis_layer_command.cpp
   commands/kis_layer_props_command.cpp
   commands/kis_node_command.cpp
   commands/kis_node_compositeop_command.cpp
   commands/kis_node_opacity_command.cpp
   commands/kis_node_move_command.cpp
   commands/kis_node_property_list_command.cpp
   commands/kis_reselect_global_selection_command.cpp
   commands/kis_reselect_local_selection_command.cpp
   commands/kis_set_global_selection_command.cpp
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
   kis_base_node.cpp
   kis_base_processor.cpp
   kis_basic_math_toolbox.cpp
   kis_bookmarked_configuration_manager.cc
   kis_clone_layer.cpp
   kis_colorspace_convert_visitor.cpp
   kis_config_widget.cpp
   kis_convolution_kernel.cc
   kis_convolution_painter.cc
   kis_effect_mask.cc
   kis_fill_painter.cc
   kis_filter_mask.cpp
   kis_filter_strategy.cc
   kis_gradient_painter.cc
   kis_gradient_job.cpp
   kis_group_layer.cc
   kis_count_visitor.cpp
   kis_histogram.cc
   kis_image.cc
   kis_iterator.cc
   kis_layer.cc
   kis_mask.cc
   kis_base_mask_generator.cpp
   kis_rect_mask_generator.cpp
   kis_circle_mask_generator.cpp
   kis_math_toolbox.cpp
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
   kis_projection.cc
   kis_properties_configuration.cc
   kis_random_accessor.cpp
   kis_random_generator.cc
   kis_random_sub_accessor.cpp
   kis_shear_visitor.cpp
   kis_selected_transaction.cc
   kis_selection.cc
   kis_selection_mask.cpp
   kis_selection_transaction.cc
   kis_serializable_configuration.cc
   kis_shared.cc
   kis_threaded_applicator.cpp
   kis_transaction.cc
   kis_transform_worker.cc
   kis_transformation_mask.cpp
   kis_transparency_mask.cc
   kis_undo_adapter.cc
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
   recorder/kis_node_query_path.cc
   recorder/kis_play_info.cc
   recorder/kis_recorded_action.cc
   recorder/kis_recorded_action_factory_registry.cc
   recorder/kis_recorded_bezier_curve_paint_action.cpp
   recorder/kis_recorded_filter_action.cpp
   recorder/kis_recorded_paint_action.cpp
   recorder/kis_recorded_polyline_paint_action.cpp
)


kde4_add_library(kritaimage SHARED ${kritaimage_LIB_SRCS})

target_link_libraries(kritaimage ${KDE4_KPARTS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kobase komain koresources pigmentcms ${KDE4_THREADWEAVER_LIBRARIES} )
target_link_libraries(kritaimage LINK_INTERFACE_LIBRARIES kobase koresources pigmentcms komain ${KDE4_KPARTS_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

if(OPENEXR_FOUND)
  target_link_libraries(kritaimage  ${OPENEXR_LIBRARIES})
endif(OPENEXR_FOUND)


set_target_properties(kritaimage PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kritaimage  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install schemas #############
install( FILES
    metadata/schemas/dc.schema
    metadata/schemas/exif.schema
    metadata/schemas/tiff.schema
    metadata/schemas/mkn.schema
    metadata/schemas/xmp.schema
    metadata/schemas/xmprights.schema
    
    DESTINATION ${DATA_INSTALL_DIR}/krita/metadata/schemas)

########### install files ###############

install( FILES 
    kis_base_node.h
    kis_base_processor.h
    kis_config_widget.h
    kis_convolution_kernel.h
    kis_convolution_painter.h
    kis_debug.h
    kis_filter_strategy.h
    kis_generic_colorspace.h
    kis_global.h
    kis_image.h
    kis_iterator.h
    kis_iterators_pixel.h
    kis_iterator_pixel_trait.h
    kis_node.h
    kis_node_facade.h
    kis_node_graph_listener.h
    kis_painter.h
    kis_paint_device.h
    kis_properties_configuration.h
    kis_processing_information.h
    kis_random_accessor.h
    kis_iterators_pixel.h
    kis_transform_worker.h
    kis_serializable_configuration.h
    kis_shared.h
    kis_shared_data.h
    kis_shared_ptr.h
    kis_shared_ptr_vector.h
    kis_transaction.h
    kis_transform_worker.h
    kis_types.h
    krita_export.h
    filter/kis_filter_configuration.h
    generator/kis_generator.h
    generator/kis_generator_registry.h
    DESTINATION ${INCLUDE_INSTALL_DIR})

#install( FILES   kis_debug.h kis_global.h kis_annotation.h
#kis_shared_ptr_vector.h kis_undo_adapter.h kis_progress_subject.h
#kis_progress_display_interface.h kis_adjustment_layer.h
#kis_autogradient_resource.h 
#kis_command.h kis_convolution_painter.h kis_fill_painter.h
#kis_filter.h  	kis_filter_registry.h kis_gradient.h
#kis_gradient_painter.h kis_histogram.h kis_image.h
# kis_iterator.h kis_iterators_pixel.h
#kis_iteratorpixeltrait.h kis_layer.h kis_meta_registry.h
#kis_nameserver.h   kis_paint_device.h kis_painter.h kis_paintop.h
#kis_paintop_registry.h  	kis_palette.h kis_pattern.h 
#kis_resource.h kis_selection.h  	kis_transaction.h kis_types.h
#kis_vec.h kis_filter_config_widget.h  	kis_filter_configuration.h
#kis_exif_info.h kis_exif_value.h kis_perspective_grid.h
#kis_perspective_math.h kis_threaded_applicator.h DESTINATION ${INCLUDE_INSTALL_DIR})

