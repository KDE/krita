include_directories(${CMAKE_SOURCE_DIR}/krita/ui/obsolete/)
set(kritaui_LIB_SRCS
    asm_scale.S
    scale.cpp
    imageviewer.cc
    kcurve.cc
    kis_autobrush.cc
    kis_autogradient.cc
    kis_birdeye_box.cc
    kis_boundary_painter.cc
    kis_brush_chooser.cc
    kis_canvas2.cpp
    kis_clipboard.cc
    kis_cmb_composite.cc
    kis_cmb_idlist.cc
    kis_color_cup.cc
    kis_config.cc
    kis_controlframe.cc
    kis_cursor.cc
    kis_custom_brush.cc
    kis_custom_image_widget.cc
    kis_custom_palette.cc
    kis_custom_pattern.cc
    kis_dlg_adj_layer_props.cc
    kis_dlg_adjustment_layer.cc
    kis_dlg_apply_profile.cc
    kis_dlg_image_properties.cc
    kis_dlg_layer_properties.cc
    kis_dlg_new_layer.cc
    kis_dlg_preferences.cc
    kis_doc2.cc
    kis_double_widget.cc
    kis_factory2.cc
    kis_filter_manager.cc
    kis_filters_listview.cc
    kis_gradient_chooser.cc
    kis_gradient_slider_widget.cc
    kis_grid_drawer.cpp
    kis_grid_manager.cpp
    kis_histogram_view.cc
    kis_icon_item.cc
    kis_iconwidget.cc
    kis_import_catcher.cc
    kis_int_spinbox.cc
    kis_itemchooser.cc
    kis_label_cursor_pos.cc
    kis_label_progress.cc
    kis_label_zoom.cc
    kis_layer_manager.cc
    kis_layerbox.cc
    kis_matrix_widget.cpp
    kis_multi_bool_filter_widget.cc
    kis_multi_double_filter_widget.cc
    kis_multi_integer_filter_widget.cc
    kis_oasis_load_data_visitor.cc
    kis_oasis_load_visitor.cc
    kis_oasis_save_data_visitor.cc
    kis_oasis_save_visitor.cc
    kis_opengl_canvas2.cpp
    kis_paintop_box.cc 
    kis_palette_view.cc
    kis_palette_widget.cc
    kis_part_layer.cc
    kis_part_layer_handler.cc
    kis_pattern_chooser.cc
    kis_perspective_grid_manager.cpp
    kis_previewdialog.cc
    kis_previewwidget.cc
    kis_qpainter_canvas.cpp
    kis_resource_mediator.cc
    kis_resource_provider.cc
    kis_resourceserver.cc
    kis_selection_manager.cc
    kis_selection_options.cc
    kis_statusbar.cc
    kis_strategy_move.cc
    kis_text_brush.cc
    kis_tool.cc
    kis_tool_freehand.cc
    kis_tool_paint.cc
    kis_tool_shape.cc
    kis_view2.cpp
    kis_view_converter.cpp
    kis_zoom_manager.cc
    kobirdeyepanel.cpp
    squeezedcombobox.cpp
    kis_image_manager.cc
    kis_mask_manager.cc
    kis_shape_layer.cc
    kis_layer_shape.cc
    kis_mask_shape.cc
    kis_layer_container_shape.cc
)


kde4_automoc(${kritaui_LIB_SRCS})

kde4_add_ui_files(kritaui_LIB_SRCS
    wdgautobrush.ui
    wdgautogradient.ui
    wdggeneralsettings.ui
    wdgperformancesettings.ui
    wdggridsettings.ui
    wdgbirdeye.ui
    wdgapplyprofile.ui
    wdgcustombrush.ui
    wdgcustompattern.ui
    wdglayerproperties.ui
    wdgcolorsettings.ui
    wdgdisplaysettings.ui
    wdgpressuresettings.ui
    wdgtabletdevicesettings.ui
    wdgtabletsettings.ui
    kis_previewwidgetbase.ui
    kis_matrix_widget.ui
    wdglayerbox.ui
    wdgpalettechooser.ui
    wdgselectionoptions.ui
    wdgshapeoptions.ui
    wdgcustompalette.ui
    wdgtextbrush.ui
    wdgnewimage.ui
)

kde4_add_library(kritaui SHARED ${kritaui_LIB_SRCS})

target_link_libraries(kritaui ${KDE4_KDE3SUPPORT_LIBS} kritaimage kofficeui )

if(HAVE_OPENGL)
    target_link_libraries(kritaui
        ${OPENGL_LIBRARIES}
        ${QT_QTOPENGL_LIBRARY}
        )
endif(HAVE_OPENGL)

if(HAVE_XINPUTEXT)
    target_link_libraries(kritaui ${X11_Xinput_LIB})
endif(HAVE_XINPUTEXT)

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
#    kis_int_spinbox.h
#    kis_layerbox.h
#    kis_move_event.h
#    kis_multi_bool_filter_widget.h
#    kis_multi_double_filter_widget.h
#    kis_multi_integer_filter_widget.h
#    kis_paintop_box.h
#    kis_palette_widget.h
#    kis_previewdialog.h
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
#    kis_filters_listview.h
#    KoInputDevice.h
#    kis_perspective_grid_manager.h
#    DESTINATION ${INCLUDE_INSTALL_DIR})
