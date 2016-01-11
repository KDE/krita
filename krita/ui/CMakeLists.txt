# Disable -Wswitch because of the extra definitions we here:
# kis_input_manager.cpp: In member function ‘virtual bool KisInputManager::eventFilter(QObject*, QEvent*)’:
# warning: case value ‘1001’ not in enumerated type ‘QEvent::Type’ [-Wswitch]
# warning: case value ‘1002’ not in enumerated type ‘QEvent::Type’ [-Wswitch]
if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(${KDE4_ENABLE_EXCEPTIONS} -Wno-switch)
endif ()

include_directories(

    ${CMAKE_SOURCE_DIR}/libs/pigment/colorprofiles
    ${CMAKE_CURRENT_SOURCE_DIR}/flake
    ${CMAKE_CURRENT_SOURCE_DIR}/pigment/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/ora
    ${CMAKE_SOURCE_DIR}/krita/image/metadata
    ${CMAKE_SOURCE_DIR}/krita/ui/qtsingleapplication
    ${CMAKE_SOURCE_DIR}/krita/ui/qtlockedfile
    ${EXIV2_INCLUDE_DIR}
    ${OCIO_INCLUDE_DIR})

add_subdirectory( tests )

if (APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
endif ()

set(kritaui_LIB_SRCS
    canvas/kis_canvas_widget_base.cpp
    canvas/kis_canvas2.cpp
    canvas/kis_canvas_updates_compressor.cpp
    canvas/kis_canvas_controller.cpp
    canvas/kis_paintop_transformation_connector.cpp
    canvas/kis_display_color_converter.cpp
    canvas/kis_display_filter.cpp
    canvas/kis_exposure_gamma_correction_interface.cpp
    canvas/kis_tool_proxy.cpp
    canvas/kis_canvas_decoration.cc
    canvas/kis_coordinates_converter.cpp
    canvas/kis_grid_manager.cpp
    canvas/kis_grid_decoration.cpp
    canvas/kis_grid_painter_configuration.cpp
    canvas/kis_perspective_grid_manager.cpp
    canvas/kis_perspective_grid_decoration.cpp
    canvas/kis_prescaled_projection.cpp
    canvas/kis_qpainter_canvas.cpp
    canvas/kis_projection_backend.cpp
    canvas/kis_update_info.cpp
    canvas/kis_image_patch.cpp
    canvas/kis_image_pyramid.cpp
    canvas/kis_infinity_manager.cpp
    dialogs/kis_about_application.cpp
    dialogs/kis_dlg_adj_layer_props.cc
    dialogs/kis_dlg_adjustment_layer.cc
    dialogs/kis_dlg_filter.cpp
    dialogs/kis_dlg_generator_layer.cpp
    dialogs/kis_dlg_file_layer.cpp
    dialogs/kis_dlg_image_properties.cc
    dialogs/kis_dlg_layer_properties.cc
    dialogs/kis_dlg_preferences.cc
    dialogs/slider_and_spin_box_sync.cpp
    dialogs/kis_dlg_blacklist_cleanup.cpp
    dialogs/kis_dlg_layer_style.cpp
    dialogs/kis_dlg_png_import.cpp
    flake/kis_node_dummies_graph.cpp
    flake/kis_dummies_facade_base.cpp
    flake/kis_dummies_facade.cpp
    flake/kis_node_shapes_graph.cpp
    flake/kis_node_shape.cpp
    flake/kis_shape_controller.cpp
    flake/kis_shape_layer.cc
    flake/kis_shape_layer_canvas.cpp
    flake/kis_shape_selection.cpp
    flake/kis_shape_selection_canvas.cpp
    flake/kis_shape_selection_model.cpp
    flake/kis_take_all_shapes_command.cpp
    kis_autogradient.cc
    kis_bookmarked_configurations_editor.cc
    kis_bookmarked_configurations_model.cc
    kis_bookmarked_filter_configurations_model.cc
    kis_canvas_resource_provider.cpp
    kis_categories_mapper.cpp
    kis_categorized_list_model.cpp
    kis_categorized_item_delegate.cpp
    kis_clipboard.cc
    kis_config.cc
    kis_config_notifier.cpp
    kis_control_frame.cpp
    kis_composite_ops_model.cc
    kis_paint_ops_model.cpp
    kis_cursor.cc
    kis_custom_pattern.cc
    kis_file_layer.cpp
    kis_safe_document_loader.cpp
    kis_splash_screen.cpp
    kis_filter_manager.cc
    kis_filters_model.cc
    kis_histogram_view.cc
    kis_image_manager.cc
    kis_image_view_converter.cpp
    kis_import_catcher.cc
    kis_layer_manager.cc
    kis_mask_manager.cc
    kis_mimedata.cpp
    kis_node_commands_adapter.cpp
    kis_node_manager.cpp
    kis_node_juggler_compressed.cpp
    kis_node_selection_adapter.cpp
    kis_node_model.cpp
    kis_model_index_converter_base.cpp
    kis_model_index_converter.cpp
    kis_model_index_converter_show_all.cpp
    kis_model_index_converter_animated_layers.cpp
    kis_painting_assistant.cc
    kis_painting_assistants_decoration.cpp
    kis_painting_assistants_manager.cpp
    kis_paintop_box.cc
    kis_paintop_option.cpp
    kis_paintop_options_model.cpp
    kis_paintop_settings_widget.cpp
    kis_popup_palette.cpp
    kis_png_converter.cpp
    kis_preference_set_registry.cpp
    kis_resource_server_provider.cpp
    kis_selection_decoration.cc
    kis_selection_manager.cc
    kis_statusbar.cc
    kis_zoom_manager.cc
    kis_favorite_resource_manager.cpp
    kis_workspace_resource.cpp
    kis_action.cpp
    kis_action_manager.cpp
    kis_view_plugin.cpp
    kis_canvas_controls_manager.cpp
    kis_tooltip_manager.cpp
    kisexiv2/kis_exif_io.cpp
    kisexiv2/kis_exiv2.cpp
    kisexiv2/kis_iptc_io.cpp
    kisexiv2/kis_xmp_io.cpp
    kra/kis_kra_utils.cpp
    kra/kis_kra_load_visitor.cpp
    kra/kis_kra_loader.cpp
    kra/kis_kra_save_visitor.cpp
    kra/kis_kra_saver.cpp
    kra/kis_kra_savexml_visitor.cpp
    opengl/kis_opengl.cpp
    opengl/kis_opengl_canvas2.cpp
    opengl/kis_opengl_canvas_debugger.cpp
    opengl/kis_opengl_image_textures.cpp
    opengl/kis_texture_tile.cpp
    opengl/kis_texture_tile_update_info.cpp
    kis_fps_decoration.cpp
    ora/kis_open_raster_stack_load_visitor.cpp
    ora/kis_open_raster_stack_save_visitor.cpp
    ora/ora_load_context.cc
    ora/ora_save_context.cc
    recorder/kis_node_query_path_editor.cc
    recorder/kis_recorded_action_creator.cc
    recorder/kis_recorded_action_creator_factory.cc
    recorder/kis_recorded_action_creator_factory_registry.cc
    recorder/kis_recorded_action_editor_factory.cc
    recorder/kis_recorded_action_editor_factory_registry.cc
    recorder/kis_recorded_filter_action_editor.cc
    recorder/kis_recorded_filter_action_creator.cpp
    recorder/kis_recorded_paint_action_editor.cc
    tool/kis_selection_tool_helper.cpp
    tool/kis_selection_tool_config_widget_helper.cpp
    tool/kis_rectangle_constraint_widget.cpp
    tool/kis_shape_tool_helper.cpp
    tool/kis_tool.cc
    tool/kis_delegated_tool_policies.cpp
    tool/kis_tool_freehand.cc
    tool/kis_speed_smoother.cpp
    tool/kis_painting_information_builder.cpp
    tool/kis_tool_freehand_helper.cpp
    tool/kis_tool_multihand_helper.cpp
    tool/kis_figure_painting_tool_helper.cpp
    tool/kis_recording_adapter.cpp
    tool/kis_tool_paint.cc
    tool/kis_tool_shape.cc
    tool/kis_tool_ellipse_base.cpp
    tool/kis_tool_rectangle_base.cpp
    tool/kis_tool_polyline_base.cpp
    tool/kis_tool_utils.cpp
    tool/kis_resources_snapshot.cpp
    tool/kis_smoothing_options.cpp
    tool/strokes/freehand_stroke.cpp
    tool/strokes/kis_painter_based_stroke_strategy.cpp
    tool/strokes/kis_filter_stroke_strategy.cpp
    widgets/kis_channelflags_widget.cpp
    widgets/kis_cmb_composite.cc
    widgets/kis_cmb_contour.cpp
    widgets/kis_cmb_gradient.cpp
    widgets/kis_paintop_list_widget.cpp
    widgets/kis_cmb_idlist.cc
    widgets/kis_color_space_selector.cc
    widgets/kis_advanced_color_space_selector.cc
    widgets/kis_cie_tongue_widget.cpp
    widgets/kis_curve_widget.cpp
    widgets/kis_custom_image_widget.cc
    widgets/kis_image_from_clipboard_widget.cpp
    widgets/kis_double_widget.cc
    widgets/kis_filter_selector_widget.cc
    widgets/kis_gradient_chooser.cc
    widgets/kis_gradient_slider_widget.cc
    widgets/kis_gradient_slider.cpp
    widgets/kis_iconwidget.cc
    widgets/kis_mask_widgets.cpp
    widgets/kis_meta_data_merge_strategy_chooser_widget.cc
    widgets/kis_multi_bool_filter_widget.cc
    widgets/kis_multi_double_filter_widget.cc
    widgets/kis_multi_integer_filter_widget.cc
    widgets/kis_multipliers_double_slider_spinbox.cpp
    widgets/kis_paintop_presets_popup.cpp
    widgets/kis_tool_options_popup.cpp
    widgets/kis_paintop_presets_chooser_popup.cpp
    widgets/kis_pattern_chooser.cc
    widgets/kis_popup_button.cc
    widgets/kis_preset_chooser.cpp
    widgets/kis_progress_widget.cpp
    widgets/kis_selection_options.cc
    widgets/kis_scratch_pad.cpp
    widgets/kis_scratch_pad_event_filter.cpp
    widgets/kis_preset_selector_strip.cpp
    widgets/kis_tree_view_popup.cc
    widgets/kis_slider_spin_box.cpp
    widgets/kis_size_group.cpp
    widgets/kis_size_group_p.cpp
    widgets/kis_wdg_generator.cpp
    widgets/kis_workspace_chooser.cpp
    widgets/squeezedcombobox.cpp
    widgets/kis_categorized_list_view.cpp
    widgets/kis_widget_chooser.cpp
    widgets/kis_tool_button.cpp
    widgets/kis_floating_message.cpp
    widgets/kis_lod_availability_widget.cpp
    input/kis_input_manager.cpp
    input/kis_input_manager_p.cpp
    input/kis_extended_modifiers_mapper.cpp
    input/kis_abstract_input_action.cpp
    input/kis_tool_invocation_action.cpp
    input/kis_pan_action.cpp
    input/kis_alternate_invocation_action.cpp
    input/kis_rotate_canvas_action.cpp
    input/kis_zoom_action.cpp
    input/kis_change_frame_action.cpp
    input/kis_gamma_exposure_action.cpp
    input/kis_show_palette_action.cpp
    input/kis_change_primary_setting_action.cpp
    input/kis_abstract_shortcut.cpp
    input/kis_single_action_shortcut.cpp
    input/kis_stroke_shortcut.cpp
    input/kis_shortcut_matcher.cpp
    input/kis_select_layer_action.cpp

    operations/kis_operation.cpp
    operations/kis_operation_configuration.cpp
    operations/kis_operation_registry.cpp
    operations/kis_operation_ui_factory.cpp
    operations/kis_operation_ui_widget.cpp
    operations/kis_filter_selection_operation.cpp
    actions/kis_selection_action_factories.cpp
    input/kis_touch_shortcut.cpp
    kis_document_undo_store.cpp
    kis_transaction_based_command.cpp
    kis_gui_context_command.cpp
    kis_gui_context_command_p.cpp
    input/kis_tablet_debugger.cpp
    input/kis_input_profile_manager.cpp
    input/kis_input_profile.cpp
    input/kis_shortcut_configuration.cpp
    input/config/kis_input_configuration_page.cpp
    input/config/kis_edit_profiles_dialog.cpp
    input/config/kis_input_profile_model.cpp
    input/config/kis_input_configuration_page_item.cpp
    input/config/kis_action_shortcuts_model.cpp
    input/config/kis_input_type_delegate.cpp
    input/config/kis_input_mode_delegate.cpp
    input/config/kis_input_button.cpp
    input/config/kis_input_editor_delegate.cpp
    input/config/kis_mouse_input_editor.cpp
    input/config/kis_wheel_input_editor.cpp
    input/config/kis_key_input_editor.cpp
    processing/fill_processing_visitor.cpp
    kis_asl_layer_style_serializer.cpp
    kis_psd_layer_style_resource.cpp
    canvas/kis_mirror_axis.cpp

    kis_abstract_perspective_grid.cpp

    KisApplication.cpp
    KisAutoSaveRecoveryDialog.cpp
    KisDetailsPane.cpp
    KisDocument.cpp
    KisDocumentEntry.cpp
    KisNodeDelegate.cpp
    KisNodeToolTip.cpp
    KisNodeView.cpp
    KisFilterChain.cpp
    KisFilterChainLink.cpp
    KisFilterChainLinkList.cpp
    KisImportExportFilter.cpp
    KisFilterEdge.cpp
    KisFilterEntry.cpp
    KisFilterGraph.cpp
    KisImportExportManager.cpp
    KisImportExportManager_p.cpp
    KisFilterVertex.cpp
    KisMainWindow.cpp
    KisOpenPane.cpp
    KisPart.cpp
    KisPrintJob.cpp
    KisTemplate.cpp
    KisTemplateCreateDia.cpp
    KisTemplateGroup.cpp
    KisTemplates.cpp
    KisTemplatesPane.cpp
    KisTemplateTree.cpp
    KisUndoStackAction.cpp
    KisView.cpp

    thememanager.cpp

    kis_mainwindow_observer.cpp
    KisViewManager.cpp
    kis_mirror_manager.cpp

    qtlockedfile/qtlockedfile.cpp

    qtsingleapplication/qtlocalpeer.cpp
    qtsingleapplication/qtsingleapplication.cpp


    kis_md5_generator.cpp
    KisApplicationArguments.cpp

    KisNetworkAccessManager.cpp
    KisWelcomeScreen.cpp
    KisMultiFeedRSSModel.cpp

    KisPaletteModel.cpp
    KisColorsetChooser.cpp
)

if(WIN32)
    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        input/kis_tablet_event.cpp
        input/wintab/kis_tablet_support_win.cpp
        input/wintab/kis_screen_size_choice_dialog.cpp
        qtlockedfile/qtlockedfile_win.cpp
        )
      include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()
if (HAVE_OPENGL)
    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        kis_animation_frame_cache.cpp
        kis_animation_cache_populator.cpp
        canvas/kis_animation_player.cpp
        kis_animation_exporter.cpp
    )
endif()

if(UNIX)
    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        input/kis_tablet_event.cpp
        input/wintab/kis_tablet_support.cpp
        qtlockedfile/qtlockedfile_unix.cpp
        )
    if(NOT APPLE)
        set(kritaui_LIB_SRCS
            ${kritaui_LIB_SRCS}
            input/wintab/kis_tablet_support_x11.cpp
            input/wintab/qxcbconnection_xi2.cpp
            input/wintab/qxcbconnection.cpp
            input/wintab/kis_xi2_event_filter.cpp
            )
    endif()
endif()

kde_enable_exceptions()

if(WIN32)
    #ki18n_wrap_ui(
    #    input/wintab/kis_screen_size_choice_dialog.ui
    #)
endif()



ki18n_wrap_ui(kritaui_LIB_SRCS
    forms/wdgdlgpngimport.ui
    forms/wdgfullscreensettings.ui
    forms/wdgautogradient.ui
    forms/wdggeneralsettings.ui
    forms/wdgperformancesettings.ui
    forms/wdggridsettings.ui
    forms/wdggenerators.ui
    forms/wdgcustompalette.ui
    forms/wdgbookmarkedconfigurationseditor.ui
    forms/wdgapplyprofile.ui
    forms/wdgcustompattern.ui
    forms/wdglayerproperties.ui
    forms/wdgcolorsettings.ui
    forms/wdgtabletsettings.ui
    forms/wdgcolorspaceselector.ui
    forms/wdgcolorspaceselectoradvanced.ui
    forms/wdgdisplaysettings.ui
    forms/kis_previewwidgetbase.ui
    forms/kis_matrix_widget.ui
    forms/wdgselectionoptions.ui
    forms/wdggeometryoptions.ui
    forms/wdgnewimage.ui
    forms/wdgimageproperties.ui
    forms/wdgmaskfromselection.ui
    forms/wdgmasksource.ui
    forms/wdgfilterdialog.ui
    forms/wdgmetadatamergestrategychooser.ui
    forms/wdgpaintoppresets.ui
    forms/wdgpaintopsettings.ui
    forms/wdgdlggeneratorlayer.ui
    forms/wdgdlgfilelayer.ui
    forms/wdgfilterselector.ui
    forms/wdgfilternodecreation.ui
    forms/wdgpaintactioneditor.ui
    forms/wdgmultipliersdoublesliderspinbox.ui
    forms/wdgnodequerypatheditor.ui
    forms/wdgpresetselectorstrip.ui
    forms/wdgdlgblacklistcleanup.ui
    forms/wdgrectangleconstraints.ui
    forms/KisDetailsPaneBase.ui
    forms/KisOpenPaneBase.ui
    input/config/kis_input_configuration_page.ui
    input/config/kis_edit_profiles_dialog.ui
    input/config/kis_input_configuration_page_item.ui
    input/config/kis_mouse_input_editor.ui
    input/config/kis_wheel_input_editor.ui
    input/config/kis_key_input_editor.ui

    layerstyles/wdgBevelAndEmboss.ui
    layerstyles/wdgblendingoptions.ui
    layerstyles/WdgColorOverlay.ui
    layerstyles/wdgContour.ui
    layerstyles/wdgdropshadow.ui
    layerstyles/WdgGradientOverlay.ui
    layerstyles/wdgInnerGlow.ui
    layerstyles/wdglayerstyles.ui
    layerstyles/WdgPatternOverlay.ui
    layerstyles/WdgSatin.ui
    layerstyles/WdgStroke.ui
    layerstyles/wdgstylesselector.ui
    layerstyles/wdgTexture.ui

    wdgsplash.ui
    input/wintab/kis_screen_size_choice_dialog.ui
)


QT5_WRAP_CPP(kritaui_HEADERS_MOC KisNodePropertyAction_p.h)

add_library(kritaui SHARED ${kritaui_HEADERS_MOC} ${kritaui_LIB_SRCS} )
generate_export_header(kritaui BASE_NAME kritaui)

target_link_libraries(kritaui KF5::CoreAddons KF5::Completion KF5::I18n KF5::ItemViews Qt5::Network
                      kritacolor kritaimage kritalibbrush kritawidgets kritawidgetutils ${PNG_LIBRARIES} ${EXIV2_LIBRARIES}
)

if (HAVE_KIO)
    target_link_libraries(kritaui KF5::KIOCore)
endif() 

if (NOT WIN32 AND NOT APPLE)
  target_link_libraries(kritaui ${X11_X11_LIB}
                                ${X11_Xinput_LIB}
                                ${XCB_LIBRARIES})
endif()

if(APPLE)
    target_link_libraries(kritaui ${FOUNDATION_LIBRARY})
endif ()


if(HAVE_OPENEXR)
    target_link_libraries(kritaui ${OPENEXR_LIBRARIES})
endif()

if(HAVE_OPENGL)
    # Add VSync disable workaround
    if(NOT WIN32 AND NOT APPLE)
      target_link_libraries(kritaui ${CMAKE_DL_LIBS} Qt5::X11Extras)
    endif()
endif()

if(X11_FOUND)
    target_link_libraries(kritaui Qt5::X11Extras ${X11_LIBRARIES})
endif()

target_link_libraries(kritaui LINK_INTERFACE_LIBRARIES kritaimage kritalibbrush kritapigment KF5::Completion KF5::I18n ${GL_INTERFACE_LIBRARIES})

set_target_properties(kritaui
    PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritaui  ${INSTALL_TARGETS_DEFAULT_ARGS})


if (APPLE)
    install(FILES osx.stylesheet DESTINATION ${DATA_INSTALL_DIR}/krita)
endif ()
