add_subdirectory( tests )
add_subdirectory( tiles )

########### next target ###############

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

# Chose a tiles backend (comment out the next line to enable the new system):
set(USE_OLD_TILESYSTEM 1)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-tiles.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/../config-tiles.h) ### WRONG PLACE???

if(USE_OLD_TILESYSTEM)
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
else(USE_OLD_TILESYSTEM)
set(libkritatile_SRCS
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tiled_random_accessor.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tiledvlineiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tiledhlineiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tileddatamanager.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tile.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tilediterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tiledrectiterator.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_memento.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tile_compressor.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tilestore.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tilestorememory.cc
	${CMAKE_SOURCE_DIR}/krita/image/tiles_new/kis_tileswapper.cc
)
add_subdirectory( tiles_new )
endif(USE_OLD_TILESYSTEM)

include_directories( ${KDE4_INCLUDE_DIR}/threadweaver/
                     ${CMAKE_SOURCE_DIR}/krita/image/metadata
                     ${KOMAIN_INCLUDES}
)

set(kritaimage_LIB_SRCS
   ${libkritatile_SRCS}
   commands/kis_image_commands.cc
   commands/kis_layer_commands.cc
   filter/kis_filter.cc
   filter/kis_filter_configuration.cc
   filter/kis_filter_config_widget.cc
   filter/kis_filter_job.cpp
   filter/kis_filter_registry.cc
   filter/kis_filter_strategy.cc
   filter/kis_recorded_filter_action.cc
   generator/kis_generator.cpp
   generator/kis_generator_registry.cpp
   kis_action_recorder.cc
   kis_adjustment_layer.cc
   kis_auto_brush.cpp
   kis_autogradient_resource.cc
   kis_base_node.cpp
   kis_base_processor.cpp
   kis_basic_math_toolbox.cpp
   kis_bookmarked_configuration_manager.cc
   kis_bottom_up_update_strategy.cpp
   kis_boundary.cc
   kis_brush.cc
   kis_brush_p.cc
   kis_brush_registry.cpp
   kis_clone_layer.cpp
   kis_complex_color.cpp
   kis_convolution_painter.cc
   kis_effect_mask.cc
   kis_fill_painter.cc
   kis_filter_mask.cpp
   kis_gradient_painter.cc
   kis_group_layer.cc
   kis_histogram.cc
   kis_image.cc
   kis_iterator.cc
   kis_layer.cc
   kis_macro.cc
   kis_mask.cc
   kis_math_toolbox.cpp
   kis_name_server.cpp
   kis_node_action.cpp
   kis_node.cpp
   kis_node_facade.cpp
   kis_paint_device.cc
   kis_painter.cc
   kis_painterly_overlay_colorspace.cpp
   kis_painterly_overlay.cpp
   kis_paint_information.cc
   kis_paint_layer.cc
   kis_paintop.cc
   kis_paintop_registry.cc
   kis_paintop_settings.cpp
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
   kis_recorded_action.cc
   kis_recorded_action_factory_registry.cc
   kis_rotate_visitor.cc
   kis_selected_transaction.cc
   kis_selection.cc
   kis_selection_mask.cpp
   kis_serializable_configuration.cc
   kis_shared.cc
   kis_threaded_applicator.cpp
   kis_transaction.cc
   kis_transformation_mask.cpp
   kis_transform_worker.cc
   kis_transparency_mask.cc
   kis_top_down_update_strategy.cpp
   kis_undo_adapter.cc
   kis_vec.cc
   metadata/kis_meta_data_entry.cc
   metadata/kis_meta_data_filter.cc
   metadata/kis_meta_data_filter_p.cc
   metadata/kis_meta_data_filter_registry.cc
   metadata/kis_meta_data_filter_registry_model.cc
   metadata/kis_meta_data_io_backend.cc
   metadata/kis_meta_data_merge_strategy.cc
   metadata/kis_meta_data_merge_strategy_p.cc
   metadata/kis_meta_data_merge_strategy_registry.cc
   metadata/kis_meta_data_schema.cc
   metadata/kis_meta_data_store.cc
   metadata/kis_meta_data_value.cc
)


kde4_add_library(kritaimage SHARED ${kritaimage_LIB_SRCS})

target_link_libraries(kritaimage ${KDE4_KDECORE_LIBS} pigmentcms komain koguiutils koresources ${KDE4_THREADWEAVER_LIBRARIES} )

set_target_properties(kritaimage PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritaimage  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############
# Don't install the header files for now
#install( FILES   kis_debug.h kis_global.h kis_annotation.h
#kis_shared_ptr_vector.h kis_undo_adapter.h kis_progress_subject.h
#kis_progress_display_interface.h kis_adjustment_layer.h
#  	kis_autobrush_resource.h
#kis_autogradient_resource.h kis_boundary.h kis_brush.h
#kis_command.h kis_convolution_painter.h kis_fill_painter.h
#kis_filter.h  	kis_filter_registry.h kis_gradient.h
#kis_gradient_painter.h kis_histogram.h kis_image.h
#kis_imagepipe_brush.h kis_iterator.h kis_iterators_pixel.h
#kis_iteratorpixeltrait.h kis_layer.h kis_meta_registry.h
#kis_nameserver.h   kis_paint_device.h kis_painter.h kis_paintop.h
#kis_paintop_registry.h  	kis_palette.h kis_pattern.h 
#kis_resource.h kis_selection.h  	kis_transaction.h kis_types.h
#kis_vec.h kis_filter_config_widget.h  	kis_filter_configuration.h
#kis_exif_info.h kis_exif_value.h kis_perspective_grid.h
#kis_perspective_math.h kis_threaded_applicator.h DESTINATION ${INCLUDE_INSTALL_DIR})

