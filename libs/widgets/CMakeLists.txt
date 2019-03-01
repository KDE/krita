add_subdirectory( tests )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kritawidgets_LIB_SRCS
    KoGradientEditWidget.cpp
    KoVBox.cpp
    KoDialog.cpp
    KoZoomWidget.cpp
    KoAspectButton.cpp
    KoPagePreviewWidget.cpp
    KoSliderCombo.cpp
    KoColorPopupButton.cpp
    KoConfigAuthorPage.cpp
    KoUnitDoubleSpinBox.cpp
    KoZoomAction.cpp
    KoZoomController.cpp
    KoZoomInput.cpp
    KoZoomHandler.cpp
    KoZoomMode.cpp
    KoDpi.cpp
    KoColorPatch.cpp
    KoColorPopupAction.cpp
    KoColorSetWidget.cpp
    KoColorSlider.cpp
    KoTriangleColorSelector.cpp
    KoResourcePopupAction.cpp
    KoLegacyResourceModel.cpp
    KoResourceTagStore.cpp
    KoRuler.cpp
    KoResourceServerAdapter.cpp
    KoResourceServerProvider.cpp
    KoLineStyleSelector.cpp
    KoLineStyleItemDelegate.cpp
    KoLineStyleModel.cpp
    KoResourceFiltering.cpp
    KoTitledTabWidget.cpp

    KoToolBoxButton.cpp
    KoToolBox.cpp
    KoToolBoxDocker.cpp
    KoToolBoxFactory.cpp
    KoToolDocker.cpp
    
    KoPageLayoutWidget.cpp
    KoPageLayoutDialog.cpp
    KoShadowConfigWidget.cpp
    KoMarkerSelector.cpp
    KoMarkerModel.cpp
    KoMarkerItemDelegate.cpp

    KoDocumentInfoDlg.cpp

    WidgetsDebug.cpp

    kis_file_name_requester.cpp

    kis_double_parse_spin_box.cpp
    kis_double_parse_unit_spin_box.cpp
    kis_int_parse_spin_box.cpp

    KisColorSelectorInterface.cpp
    KoAnchorSelectionWidget.cpp

    KisGradientSlider.cpp
    KisGradientSliderWidget.cpp

    kis_color_input.cpp

    # classes used by internal color selector
    kis_spinbox_color_selector.cpp
    KisVisualColorSelector.cpp
    KisVisualColorSelectorShape.cpp
    KisVisualEllipticalSelectorShape.cpp
    KisVisualRectangleSelectorShape.cpp
    KisVisualTriangleSelectorShape.cpp
    KisScreenColorPickerBase.cpp
    KisDlgInternalColorSelector.cpp

    KisPaletteModel.cpp
    KisPaletteDelegate.cpp
    kis_palette_view.cpp
    KisPaletteListWidget.cpp
    KisPaletteComboBox.cpp

    kis_popup_button.cc
    kis_color_button.cpp
)

ki18n_wrap_ui( kritawidgets_LIB_SRCS
    KoConfigAuthorPage.ui
    koDocumentInfoAboutWidget.ui
    koDocumentInfoAuthorWidget.ui
    wdg_file_name_requester.ui
    KoPageLayoutWidget.ui
    KoShadowConfigWidget.ui
    WdgDlgInternalColorSelector.ui
    WdgPaletteListWidget.ui
)

add_library(kritawidgets SHARED ${kritawidgets_LIB_SRCS})
generate_export_header(kritawidgets BASE_NAME kritawidgets)
target_link_libraries(kritawidgets
    kritaodf
    kritaglobal
    kritaflake
    kritapigment
    kritawidgetutils
    kritaresources
    kritaresourcewidgets
    Qt5::PrintSupport
    KF5::CoreAddons
    KF5::ConfigGui
    KF5::GuiAddons
    KF5::WidgetsAddons
    KF5::ConfigCore
    KF5::Completion
)

if(X11_FOUND)
    target_link_libraries(kritawidgets Qt5::X11Extras ${X11_LIBRARIES})
endif()

set_target_properties(kritawidgets PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritawidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})
