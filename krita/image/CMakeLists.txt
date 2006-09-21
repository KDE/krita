

add_subdirectory( tiles )

if(KUNITTEST_FOUND)
	add_subdirectory(tests)
endif(KUNITTEST_FOUND)



########### next target ###############

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
)


set(kritaimage_LIB_SRCS
   ${libkritatile_SRCS}
   kis_adjustment_layer.cc
   kis_progress_subject.cc
   kis_alpha_mask.cc
   kis_autobrush_resource.cc
   kis_autogradient_resource.cc
   kis_background.cc
   kis_boundary.cc
   kis_brush.cc
   kis_command.cc
   kis_convolution_painter.cc
   kis_fill_painter.cc
   kis_filter.cc
   kis_filter_registry.cc
   kis_filter_strategy.cc
   kis_filter_configuration.cc
   kis_filter_config_widget.cc
   kis_gradient.cc
   kis_gradient_painter.cc
   kis_histogram.cc
   kis_image.cc
   kis_imagepipe_brush.cc
   kis_iterator.cc
   kis_iterators_pixel.cc
   kis_layer.cc
   kis_group_layer.cc
   kis_paint_layer.cc
   kis_meta_registry.cc
   kis_nameserver.cc
   kis_painter.cc
   kis_paintop.cc
   kis_paintop_registry.cc
   kis_palette.cc
   kis_pattern.cc
   kis_random_accessor.cpp
   kis_random_sub_accessor.cpp
   kis_rect.cc
   kis_resource.cc
   kis_rotate_visitor.cc
   kis_selected_transaction.cc
   kis_selection.cc
   kis_strategy_move.cc
   kis_transaction.cc
   kis_transform_worker.cc
   kis_vec.cc
   kis_paint_device.cc
   kis_basic_math_toolbox.cpp
   kis_math_toolbox.cpp
   kis_exif_info.cc
   kis_thread_pool.cc
   kis_exif_value.cc
   kis_filter_strategy.h )

kde4_automoc(${kritaimage_LIB_SRCS})

kde4_add_library(kritaimage SHARED ${kritaimage_LIB_SRCS})

target_link_libraries(kritaimage ${KDE4_KDECORE_LIBS} kritacolor kofficecore kopainter)

set_target_properties(kritaimage PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritaimage  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES  kis_canvas_subject.h kis_canvas_controller.h kis_canvas_observer.h kis_debug_areas.h kis_global.h kis_annotation.h kis_shared_ptr_vector.h kis_undo_adapter.h kis_progress_subject.h kis_progress_display_interface.h kis_adjustment_layer.h kis_alpha_mask.h  	kis_autobrush_resource.h kis_autogradient_resource.h kis_background.h kis_boundary.h kis_brush.h  	kis_command.h kis_convolution_painter.h kis_fill_painter.h kis_filter.h  	kis_filter_registry.h kis_gradient.h kis_gradient_painter.h kis_histogram.h kis_image.h  	kis_imagepipe_brush.h kis_iterator.h kis_iterators_pixel.h  	kis_iteratorpixeltrait.h kis_layer.h kis_meta_registry.h kis_nameserver.h   kis_paint_device.h kis_painter.h kis_paintop.h kis_paintop_registry.h  	kis_palette.h kis_pattern.h kis_point.h kis_rect.h kis_resource.h kis_selection.h  	kis_transaction.h kis_types.h kis_vec.h kis_filter_config_widget.h  	kis_filter_configuration.h kis_exif_info.h kis_exif_value.h  DESTINATION ${INCLUDE_INSTALL_DIR})

