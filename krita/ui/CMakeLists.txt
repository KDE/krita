
include_directories( ${KOTEXT_INCLUDES} ${BLITZ_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/openrastertk ${CMAKE_SOURCE_DIR}/krita/image/metadata )

add_subdirectory( tests )

configure_file(config-processor.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-processor.h )

include(kdefx.cmake)

set(kritaui_LIB_SRCS
    canvas/kis_abstract_canvas_widget.cpp 
    canvas/kis_canvas2.cpp 
    canvas/kis_canvas_decoration.cc
    canvas/kis_grid_manager.cpp 
    canvas/kis_grid_painter_configuration.cpp
    canvas/kis_perspective_grid_manager.cpp  
    canvas/kis_prescaled_projection.cpp
    canvas/kis_qpainter_canvas.cpp
    dialogs/kis_dlg_adj_layer_props.cc 
    dialogs/kis_dlg_adjustment_layer.cc
    dialogs/kis_dlg_apply_profile.cc 
    dialogs/kis_dlg_filter.cpp
    dialogs/kis_dlg_generator_layer.cpp
    dialogs/kis_dlg_image_properties.cc
    dialogs/kis_dlg_layer_properties.cc
    dialogs/kis_dlg_new_layer.cc 
    dialogs/kis_dlg_preferences.cc
    dialogs/kis_dlg_transformation_effect.cpp
    kis_autogradient.cc
    kis_birdeye_box.cc 
    kis_bookmarked_configurations_editor.cc
    kis_bookmarked_configurations_model.cc
    kis_bookmarked_filter_configurations_model.cc
    kis_boundary_painter.cc 
    kis_canvas_resource_provider.cpp 
    kis_clipboard.cc 
    kis_config.cc
    kis_config_notifier.cpp 
    kis_control_frame.cpp 
    kis_cursor.cc
    kis_custom_brush.cc 
    kis_custom_palette.cc 
    kis_custom_pattern.cc
    kis_doc2.cc 
    kis_exposure_visitor.cc 
    kis_factory2.cc
    kis_filter_handler.cc 
    kis_filter_manager.cc 
    kis_filters_model.cc
    kis_histogram_view.cc 
    kis_icon_item.cc 
    kis_image_manager.cc
    kis_imagepipe_brush.cc 
    kis_image_view_converter.cpp
    kis_import_catcher.cc 
    kis_kra_loader.cpp 
    kis_kra_load_visitor.cpp
    kis_kra_save_visitor.cpp 
    kis_label_cursor_pos.cc
    kis_layer_box.cpp 
    kis_layer_container_shape.cc 
    kis_layer_manager.cc
    kis_layermap_visitor.cc 
    kis_layer_shape.cc 
    kis_mask_manager.cc
    kis_mask_shape.cc 
    kis_node_manager.cpp 
    kis_node_model.cpp
    kis_painting_assistant.cc 
    kis_paintop_box.cc 
    kis_palette_docker.cc
    kis_png_converter.cpp
    kis_print_job.cpp 
    kis_recorded_paint_actions.cc 
    kis_resource_mediator.cc
    kis_resource_server_provider.cpp 
    kis_savexml_visitor.cc
    kis_selection_manager.cc 
    kis_selection_options.cc
    kis_selection_tool_helper.cpp 
    kis_shape_controller.cpp
    kis_shape_layer_canvas.cpp 
    kis_shape_layer.cc 
    kis_shape_selection.cpp
    kis_shape_selection_model.cpp 
    kis_statusbar.cc 
    kis_text_brush.cc
    kis_tool.cc 
    kis_tool_freehand.cc 
    kis_tool_freehand_p.cpp
    kis_tool_paint.cc 
    kis_tool_shape.cc kis_view2.cpp 
    kis_zoom_manager.cc
    kobirdeyepanel.cpp 
    modeltest.cpp 
    opengl/kis_opengl_canvas2.cpp
    opengl/kis_opengl.cpp 
    opengl/kis_opengl_image_textures.cpp
    openrastertk/kis_open_raster_stack_load_visitor.cpp
    openrastertk/kis_open_raster_stack_save_visitor.cpp
    widgets/imageviewer.cc 
    widgets/kcurve.cc
    widgets/kis_auto_brush_widget.cpp 
    widgets/kis_brush_chooser.cc
    widgets/kis_channelflags_widget.cpp 
    widgets/kis_cmb_composite.cc
    widgets/kis_cmb_idlist.cc 
    widgets/kis_color_space_selector.cc
    widgets/kis_custom_image_widget.cc 
    widgets/kis_double_widget.cc
    widgets/kis_filter_selector_widget.cc
    widgets/kis_gradient_chooser.cc 
    widgets/kis_gradient_slider_widget.cc
    widgets/kis_iconwidget.cc 
    widgets/kis_item_chooser.cpp
    widgets/kis_mask_widgets.cpp 
    widgets/kis_meta_data_merge_strategy_chooser_widget.cc
    widgets/kis_multi_bool_filter_widget.cc
    widgets/kis_multi_double_filter_widget.cc
    widgets/kis_multi_integer_filter_widget.cc
    widgets/kis_paintop_presets_widget.cpp 
    widgets/kis_pattern_chooser.cc
    widgets/kis_popup_button.cc 
    widgets/kis_preset_widget.cpp 
    widgets/kis_previewwidget.cc
    widgets/kis_tree_view_popup.cc 
    widgets/kis_wdg_generator.cpp
    widgets/squeezedcombobox.cpp
)

if(HAVE_OPENGL AND HAVE_GLEW)
    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        opengl/kis_opengl_shader.cpp
        opengl/kis_opengl_fragment_shader.cpp
        opengl/kis_opengl_vertex_shader.cpp
        opengl/kis_opengl_program.cpp
        opengl/kis_opengl_hdr_exposure_program.cpp
        opengl/kis_opengl_gradient_program.cpp
        )
endif(HAVE_OPENGL AND HAVE_GLEW)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})


kde4_add_ui_files(kritaui_LIB_SRCS
    forms/wdgautobrush.ui
    forms/wdgautogradient.ui
    forms/wdggeneralsettings.ui
    forms/wdgperformancesettings.ui
    forms/wdggridsettings.ui
    forms/wdgbirdeye.ui 
    forms/wdggenerators.ui
    forms/wdgbookmarkedconfigurationseditor.ui
    forms/wdgapplyprofile.ui
    forms/wdgcustombrush.ui
    forms/wdgcustompattern.ui
    forms/wdglayerproperties.ui
    forms/wdgcolorsettings.ui
    forms/wdgcolorspaceselector.ui
    forms/wdgdisplaysettings.ui
    forms/kis_previewwidgetbase.ui
    forms/kis_matrix_widget.ui
    forms/wdglayerbox.ui
    forms/wdgpalettechooser.ui
    forms/wdgselectionoptions.ui
    forms/wdgshapeoptions.ui
    forms/wdgcustompalette.ui
    forms/wdgtextbrush.ui
    forms/wdgnewimage.ui
    forms/wdgimageproperties.ui
    forms/wdgtransformationeffect.ui
    forms/wdgmaskfromselection.ui
    forms/wdgmasksource.ui
    forms/wdgfilterdialog.ui
    forms/wdgmetadatamergestrategychooser.ui
    forms/wdgpaintoppresets.ui
    forms/wdgdlggeneratorlayer.ui
    forms/wdgfilterselector.ui
    forms/wdgfilternodecreation.ui
)

kde4_add_library(kritaui SHARED ${kritaui_LIB_SRCS} )

target_link_libraries(kritaui ${KDE4_KDE3SUPPORT_LIBS} ${BLITZ_LIBRARIES} kritaimage koguiutils kotext ${PNG_LIBRARIES})

if(HAVE_OPENGL)
    target_link_libraries(kritaui ${OPENGL_LIBRARIES} ${QT_QTOPENGL_LIBRARY})
    if(HAVE_GLEW)
        target_link_libraries(kritaui ${GLEW_LIBRARIES})
    endif(HAVE_GLEW)
endif(HAVE_OPENGL)

set_target_properties(kritaui PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kritaui  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

#install( FILES
#    kis_cmb_composite.h
#    kis_cmb_idlist.h
#    kis_color_cup.h
#    kis_config.h
#    kis_cursor.h
#    kis_double_click_event.h
#    kis_double_widget.h
#    kis_filter_manager.h
#    kis_gradient_chooser.h
#    kis_gradient_slider_widget.h
#    kis_histogram_view.h
#    kis_icon_item.h
#    kis_iconwidget.h
#    kis_itemchooser.h
#    kis_label_zoom.h
#    kis_layerbox.h
#    kis_move_event.h
#    kis_multi_bool_filter_widget.h
#    kis_multi_double_filter_widget.h
#    kis_multi_integer_filter_widget.h
#    kis_paintop_box.h
#    kis_palette_widget.h
#    kis_previewwidget.h
#    kis_tool.h
#    kis_tool_non_paint.h
#    kis_tool_paint.h
#    kis_tool_freehand.h
#    kis_tool_dummy.h
#    kis_tool_manager.h
#    kis_tool_types.h
#    kis_view2.h
#    kobirdeyepanel.h
#    KoInputDevice.h
#    canvas/kis_perspective_grid_manager.h
#    DESTINATION ${INCLUDE_INSTALL_DIR})
