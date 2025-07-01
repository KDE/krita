add_subdirectory( tests )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kritawidgets_LIB_SRCS
    KoVBox.cpp
    KoDialog.cpp
    KoZoomWidget.cpp
    KoAspectButton.cpp
    KoSliderCombo.cpp
    KoColorPopupButton.cpp
    KoConfigAuthorPage.cpp
    KoUnitDoubleSpinBox.cpp
    KoZoomAction.cpp
    KoZoomActionState.cpp
    KoZoomInput.cpp
    KoZoomHandler.cpp
    KoColorPatch.cpp
    KoColorPopupAction.cpp
    KoColorSetWidget.cpp
    KoColorSlider.cpp
    KisHsvColorSlider.cpp
    KoTriangleColorSelector.cpp
    KoResourcePopupAction.cpp
    KoRuler.cpp
    KoResourceServerProvider.cpp
    KoLineStyleSelector.cpp
    KoLineStyleItemDelegate.cpp
    KoLineStyleModel.cpp

    KoDockWidgetTitleBar.cpp
    KoDockWidgetTitleBarButton.cpp
    
    KoTitledTabWidget.cpp

    KoToolDocker.cpp
    
    KoShadowConfigWidget.cpp
    KoMarkerSelector.cpp
    KoMarkerModel.cpp
    KoMarkerItemDelegate.cpp

    WidgetsDebug.cpp

    kis_file_name_requester.cpp

    KisColorSelectorInterface.cpp
    KoAnchorSelectionWidget.cpp

    KisLevelsSlider.cpp

    kis_color_input.cpp

    # classes used by internal color selector
    kis_spinbox_color_selector.cpp
    KisSpinboxHSXSelector.cpp
    KisVisualColorModel.cpp
    KisVisualColorSelector.cpp
    KisVisualColorSelectorShape.cpp
    KisVisualDiamondSelectorShape.cpp
    KisVisualEllipticalSelectorShape.cpp
    KisVisualRectangleSelectorShape.cpp
    KisVisualTriangleSelectorShape.cpp
    KisScreenColorSamplerBase.cpp
    KisDlgInternalColorSelector.cpp

    KisPaletteModel.cpp
    KisPaletteDelegate.cpp
    kis_palette_view.cpp
    KisPaletteChooser.cpp
    KisPaletteComboBox.cpp

    kis_color_button.cpp

    KisAngleGauge.cpp
    KisAngleSelector.cpp

    KisTagSelectionWidget.cpp

    kis_warning_block.cpp
    KisWidgetConnectionUtils.cpp
    kis_spacing_selection_widget.cpp
    kis_stacked_widget.cpp

    KisStretchedIconButton.cpp
)

ki18n_wrap_ui( kritawidgets_LIB_SRCS
    KoConfigAuthorPage.ui
    wdg_file_name_requester.ui
    KoShadowConfigWidget.ui
    WdgDlgInternalColorSelector.ui
    WdgPaletteListWidget.ui
)

kis_add_library(kritawidgets SHARED ${kritawidgets_LIB_SRCS})
generate_export_header(kritawidgets BASE_NAME kritawidgets)
target_link_libraries(kritawidgets
    
    kritaglobal
    kritaflake
    kritapigment
    kritawidgetutils
    kritaresources
    kritaresourcewidgets
    Qt${QT_MAJOR_VERSION}::PrintSupport
    KF${KF_MAJOR}::CoreAddons
    KF${KF_MAJOR}::ConfigGui
    KF${KF_MAJOR}::GuiAddons
    KF${KF_MAJOR}::WidgetsAddons
    KF${KF_MAJOR}::ConfigCore
    KF${KF_MAJOR}::Completion
)

set_target_properties(kritawidgets PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritawidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})
