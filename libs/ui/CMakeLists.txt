include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/qtlockedfile
)

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
    ${OCIO_INCLUDE_DIR}
)

add_subdirectory( tests )

if (APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(APPKIT_LIBRARY AppKit)
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
    canvas/kis_grid_config.cpp
    canvas/kis_prescaled_projection.cpp
    canvas/kis_qpainter_canvas.cpp
    canvas/kis_projection_backend.cpp
    canvas/kis_update_info.cpp
    canvas/kis_image_patch.cpp
    canvas/kis_image_pyramid.cpp
    canvas/kis_infinity_manager.cpp
    canvas/kis_change_guides_command.cpp
    canvas/kis_guides_decoration.cpp
    canvas/kis_guides_manager.cpp
    canvas/kis_guides_config.cpp
    canvas/kis_snap_config.cpp
    canvas/kis_snap_line_strategy.cpp
    canvas/KisSnapPointStrategy.cpp
	canvas/KisMirrorAxisConfig.cpp
	dialogs/kis_about_application.cpp
    dialogs/kis_dlg_adj_layer_props.cc
    dialogs/kis_dlg_adjustment_layer.cc
    dialogs/kis_dlg_filter.cpp
    dialogs/kis_dlg_generator_layer.cpp
    dialogs/kis_dlg_file_layer.cpp
    dialogs/kis_dlg_filter.cpp
    dialogs/kis_dlg_stroke_selection_properties.cpp
    dialogs/kis_dlg_image_properties.cc
    dialogs/kis_dlg_layer_properties.cc
    dialogs/kis_dlg_preferences.cc
    dialogs/slider_and_spin_box_sync.cpp
    dialogs/kis_dlg_blacklist_cleanup.cpp
    dialogs/kis_dlg_layer_style.cpp
    dialogs/kis_dlg_png_import.cpp
    dialogs/kis_dlg_import_image_sequence.cpp
    dialogs/kis_delayed_save_dialog.cpp
    dialogs/KisSessionManagerDialog.cpp
    dialogs/KisNewWindowLayoutDialog.cpp
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
    brushhud/kis_uniform_paintop_property_widget.cpp
    brushhud/kis_brush_hud.cpp
    brushhud/kis_round_hud_button.cpp
    brushhud/kis_dlg_brush_hud_config.cpp
    brushhud/kis_brush_hud_properties_list.cpp
    brushhud/kis_brush_hud_properties_config.cpp

    kis_aspect_ratio_locker.cpp
    kis_autogradient.cc
    kis_bookmarked_configurations_editor.cc
    kis_bookmarked_configurations_model.cc
    kis_bookmarked_filter_configurations_model.cc
    KisPaintopPropertiesBase.cpp
    kis_canvas_resource_provider.cpp
    kis_derived_resources.cpp
    kis_categories_mapper.cpp
    kis_categorized_list_model.cpp
    kis_categorized_item_delegate.cpp
    kis_clipboard.cc
    kis_config.cc
    KisOcioConfiguration.cpp
    kis_control_frame.cpp
    kis_composite_ops_model.cc
    kis_paint_ops_model.cpp
    kis_cursor.cc
    kis_cursor_cache.cpp
    kis_custom_pattern.cc
    kis_file_layer.cpp
    kis_change_file_layer_command.h
    kis_safe_document_loader.cpp
    kis_splash_screen.cpp
    kis_filter_manager.cc
    kis_filters_model.cc
    kis_histogram_view.cc
    KisImageBarrierLockerWithFeedback.cpp
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
    kis_node_insertion_adapter.cpp
    KisNodeDisplayModeAdapter.cpp
    kis_node_model.cpp
    kis_node_filter_proxy_model.cpp
    kis_model_index_converter_base.cpp
    kis_model_index_converter.cpp
    kis_model_index_converter_show_all.cpp
    kis_painting_assistant.cc
    kis_painting_assistants_decoration.cpp
    KisDecorationsManager.cpp
    kis_paintop_box.cc
    kis_paintop_option.cpp
    kis_paintop_options_model.cpp
    kis_paintop_settings_widget.cpp
    kis_popup_palette.cpp
    kis_png_converter.cpp
    kis_preference_set_registry.cpp
    KisResourceServerProvider.cpp
    KisResourceBundleServerProvider.cpp
    KisSelectedShapesProxy.cpp
    kis_selection_decoration.cc
    kis_selection_manager.cc
    KisSelectionActionsAdapter.cpp
    kis_statusbar.cc
    kis_zoom_manager.cc
    kis_favorite_resource_manager.cpp
    kis_workspace_resource.cpp
    kis_action.cpp
    kis_action_manager.cpp
    KisActionPlugin.cpp
    kis_canvas_controls_manager.cpp
    kis_tooltip_manager.cpp
    kis_multinode_property.cpp
    kis_stopgradient_editor.cpp
    KisWelcomePageWidget.cpp

    kisexiv2/kis_exif_io.cpp
    kisexiv2/kis_exiv2.cpp
    kisexiv2/kis_iptc_io.cpp
    kisexiv2/kis_xmp_io.cpp

    opengl/kis_opengl.cpp
    opengl/kis_opengl_canvas2.cpp
    opengl/kis_opengl_canvas_debugger.cpp
    opengl/kis_opengl_image_textures.cpp
    opengl/kis_texture_tile.cpp
    opengl/kis_opengl_shader_loader.cpp
    opengl/kis_texture_tile_info_pool.cpp
    opengl/KisOpenGLUpdateInfoBuilder.cpp
    opengl/KisOpenGLModeProber.cpp
    opengl/KisScreenInformationAdapter.cpp
    kis_fps_decoration.cpp

    tool/KisToolChangesTracker.cpp
    tool/KisToolChangesTrackerData.cpp
    tool/kis_selection_tool_helper.cpp
    tool/kis_selection_tool_config_widget_helper.cpp
    tool/kis_rectangle_constraint_widget.cpp
    tool/kis_shape_tool_helper.cpp
    tool/kis_tool.cc
    tool/kis_delegated_tool_policies.cpp
    tool/kis_tool_freehand.cc
    tool/kis_speed_smoother.cpp
    tool/kis_painting_information_builder.cpp
    tool/kis_stabilized_events_sampler.cpp
    tool/kis_tool_freehand_helper.cpp
    tool/kis_tool_multihand_helper.cpp
    tool/kis_figure_painting_tool_helper.cpp
    tool/kis_tool_paint.cc
    tool/kis_tool_shape.cc
    tool/kis_tool_ellipse_base.cpp
    tool/kis_tool_rectangle_base.cpp
    tool/kis_tool_polyline_base.cpp
    tool/kis_tool_utils.cpp
    tool/kis_resources_snapshot.cpp
    tool/kis_smoothing_options.cpp
    tool/KisStabilizerDelayedPaintHelper.cpp
    tool/KisStrokeSpeedMonitor.cpp
    tool/strokes/freehand_stroke.cpp
    tool/strokes/KisStrokeEfficiencyMeasurer.cpp
    tool/strokes/kis_painter_based_stroke_strategy.cpp
    tool/strokes/kis_filter_stroke_strategy.cpp
    tool/strokes/kis_color_picker_stroke_strategy.cpp
    tool/strokes/KisFreehandStrokeInfo.cpp
    tool/strokes/KisMaskedFreehandStrokePainter.cpp
    tool/strokes/KisMaskingBrushRenderer.cpp
    tool/strokes/KisMaskingBrushCompositeOpFactory.cpp
    tool/strokes/move_stroke_strategy.cpp
    tool/KisSelectionToolFactoryBase.cpp
    tool/KisToolPaintFactoryBase.cpp

    widgets/kis_cmb_composite.cc
    widgets/kis_cmb_contour.cpp
    widgets/kis_cmb_gradient.cpp
    widgets/kis_paintop_list_widget.cpp
    widgets/kis_cmb_idlist.cc
    widgets/kis_color_space_selector.cc
    widgets/kis_advanced_color_space_selector.cc
    widgets/kis_cie_tongue_widget.cpp
    widgets/kis_tone_curve_widget.cpp
    widgets/kis_curve_widget.cpp
    widgets/kis_custom_image_widget.cc
    widgets/kis_image_from_clipboard_widget.cpp
    widgets/kis_double_widget.cc
    widgets/kis_filter_selector_widget.cc
    widgets/kis_gradient_chooser.cc
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
    widgets/kis_paintop_presets_save.cpp
    widgets/kis_paintop_preset_icon_library.cpp
    widgets/kis_pattern_chooser.cc

    widgets/kis_preset_chooser.cpp
    widgets/kis_progress_widget.cpp
    widgets/kis_selection_options.cc
    widgets/kis_scratch_pad.cpp
    widgets/kis_scratch_pad_event_filter.cpp
    widgets/kis_preset_selector_strip.cpp
    widgets/kis_slider_spin_box.cpp
    widgets/KisSelectionPropertySlider.cpp
    widgets/kis_size_group.cpp
    widgets/kis_size_group_p.cpp
    widgets/kis_wdg_generator.cpp
    widgets/kis_workspace_chooser.cpp
    widgets/kis_categorized_list_view.cpp
    widgets/kis_widget_chooser.cpp
    widgets/kis_tool_button.cpp
    widgets/kis_floating_message.cpp
    widgets/kis_lod_availability_widget.cpp
    widgets/kis_color_label_selector_widget.cpp
    widgets/kis_color_filter_combo.cpp
    widgets/kis_elided_label.cpp
    widgets/kis_stopgradient_slider_widget.cpp
    widgets/kis_preset_live_preview_view.cpp
    widgets/KisScreenColorPicker.cpp
    widgets/KoDualColorButton.cpp
    widgets/KoStrokeConfigWidget.cpp
    widgets/KoFillConfigWidget.cpp
    widgets/KisLayerStyleAngleSelector.cpp
    widgets/KisMemoryReportButton.cpp

    KisPaletteEditor.cpp
    dialogs/KisDlgPaletteEditor.cpp

    widgets/KisNewsWidget.cpp
    widgets/KisGamutMaskToolbar.cpp

    utils/kis_document_aware_spin_box_unit_manager.cpp

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
    input/kis_native_gesture_shortcut.cpp
    input/kis_single_action_shortcut.cpp
    input/kis_stroke_shortcut.cpp
    input/kis_shortcut_matcher.cpp
    input/kis_select_layer_action.cpp
    input/KisQtWidgetsTweaker.cpp
    input/KisInputActionGroup.cpp

    operations/kis_operation.cpp
    operations/kis_operation_configuration.cpp
    operations/kis_operation_registry.cpp
    operations/kis_operation_ui_factory.cpp
    operations/kis_operation_ui_widget.cpp
    operations/kis_filter_selection_operation.cpp
    actions/kis_selection_action_factories.cpp
    actions/KisPasteActionFactory.cpp
    actions/KisTransformToolActivationCommand.cpp
    input/kis_touch_shortcut.cpp
    kis_document_undo_store.cpp
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
    KisCloneDocumentStroke.cpp
    kis_node_view_color_scheme.cpp
    KisImportExportFilter.cpp
    KisFilterEntry.cpp
    KisImportExportManager.cpp
    KisImportExportUtils.cpp
    kis_async_action_feedback.cpp
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
    KisUndoActionsUpdateManager.cpp
    KisView.cpp

    thememanager.cpp

    kis_mainwindow_observer.cpp
    KisViewManager.cpp
    kis_mirror_manager.cpp

    qtlockedfile/qtlockedfile.cpp

    qtsingleapplication/qtlocalpeer.cpp
    qtsingleapplication/qtsingleapplication.cpp

    KisResourceBundle.cpp
    KisResourceBundleManifest.cpp

    kis_md5_generator.cpp
    KisApplicationArguments.cpp

    KisNetworkAccessManager.cpp
    KisMultiFeedRSSModel.cpp
    KisRemoteFileFetcher.cpp

    KisSaveGroupVisitor.cpp
    KisWindowLayoutResource.cpp
    KisWindowLayoutManager.cpp
    KisSessionResource.cpp

    KisReferenceImagesDecoration.cpp
    KisReferenceImage.cpp
    flake/KisReferenceImagesLayer.cpp
    flake/KisReferenceImagesLayer.h

)

if(WIN32)
    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        input/wintab/kis_tablet_support_win.cpp
        input/wintab/kis_screen_size_choice_dialog.cpp
        qtlockedfile/qtlockedfile_win.cpp
        input/wintab/kis_tablet_support_win8.cpp
        )
endif()

    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        kis_animation_frame_cache.cpp
        kis_animation_cache_populator.cpp
        KisAsyncAnimationRendererBase.cpp
        KisAsyncAnimationCacheRenderer.cpp
        KisAsyncAnimationFramesSavingRenderer.cpp
        dialogs/KisAsyncAnimationRenderDialogBase.cpp
        dialogs/KisAsyncAnimationCacheRenderDialog.cpp
        dialogs/KisAsyncAnimationFramesSaveDialog.cpp
        canvas/kis_animation_player.cpp
        kis_animation_importer.cpp
        KisSyncedAudioPlayback.cpp
        KisFrameDataSerializer.cpp
        KisFrameCacheStore.cpp
        KisFrameCacheSwapper.cpp
        KisAbstractFrameCacheSwapper.cpp
        KisInMemoryFrameCacheSwapper.cpp

        input/wintab/drawpile_tablettester/tablettester.cpp
        input/wintab/drawpile_tablettester/tablettest.cpp
    )

if (UNIX)
    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        qtlockedfile/qtlockedfile_unix.cpp
        )
endif()

if(APPLE)
    set(kritaui_LIB_SRCS
        ${kritaui_LIB_SRCS}
        osx.mm
    )
endif()

ki18n_wrap_ui(kritaui_LIB_SRCS
    widgets/KoFillConfigWidget.ui
    widgets/KoStrokeConfigWidget.ui
    forms/wdgdlgpngimport.ui
    forms/wdgfullscreensettings.ui
    forms/wdgautogradient.ui
    forms/wdggeneralsettings.ui
    forms/wdgperformancesettings.ui
    forms/wdggenerators.ui
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
    forms/wdgmultipliersdoublesliderspinbox.ui
    forms/wdgnodequerypatheditor.ui
    forms/wdgpresetselectorstrip.ui
    forms/wdgsavebrushpreset.ui
    forms/wdgpreseticonlibrary.ui
    forms/wdgdlgblacklistcleanup.ui
    forms/wdgrectangleconstraints.ui
    forms/wdgimportimagesequence.ui
    forms/wdgstrokeselectionproperties.ui
    forms/KisDetailsPaneBase.ui
    forms/KisOpenPaneBase.ui
    forms/wdgstopgradienteditor.ui
    forms/wdgsessionmanager.ui
    forms/wdgnewwindowlayout.ui
    forms/KisWelcomePage.ui
    forms/WdgDlgPaletteEditor.ui
    forms/KisNewsPage.ui
    forms/wdgGamutMaskToolbar.ui

    brushhud/kis_dlg_brush_hud_config.ui
    dialogs/kis_delayed_save_dialog.ui
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
    layerstyles/wdgKisLayerStyleAngleSelector.ui

    wdgsplash.ui
    input/wintab/kis_screen_size_choice_dialog.ui
    input/wintab/drawpile_tablettester/tablettest.ui

)


add_library(kritaui SHARED ${kritaui_HEADERS_MOC} ${kritaui_LIB_SRCS} )
generate_export_header(kritaui BASE_NAME kritaui)

target_link_libraries(kritaui KF5::CoreAddons KF5::Completion KF5::I18n KF5::ItemViews Qt5::Network
                      kritaimpex kritacolor kritaimage kritalibbrush kritawidgets kritawidgetutils ${PNG_LIBRARIES} LibExiv2::LibExiv2
)

if (HAVE_QT_MULTIMEDIA)
    target_link_libraries(kritaui Qt5::Multimedia)
endif()

if (NOT WIN32 AND NOT APPLE)
  target_link_libraries(kritaui ${X11_X11_LIB}
                                ${X11_Xinput_LIB}
                                ${XCB_LIBRARIES})
endif()

if(APPLE)
    target_link_libraries(kritaui ${FOUNDATION_LIBRARY})
    target_link_libraries(kritaui ${APPKIT_LIBRARY})
endif ()


target_link_libraries(kritaui ${OPENEXR_LIBRARIES})

# Add VSync disable workaround
if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(kritaui ${CMAKE_DL_LIBS} Qt5::X11Extras)
endif()

if(X11_FOUND)
    target_link_libraries(kritaui Qt5::X11Extras ${X11_LIBRARIES})
endif()

target_include_directories(kritaui
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/canvas>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/flake>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ora>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tool>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/input/wintab>
)

set_target_properties(kritaui
    PROPERTIES VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaui  ${INSTALL_TARGETS_DEFAULT_ARGS})


if (APPLE)
    install(FILES osx.stylesheet DESTINATION ${DATA_INSTALL_DIR}/krita)
endif ()
