add_subdirectory( tests )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kritawidgets_LIB_SRCS
    KoGradientEditWidget.cpp
    KoVBox.cpp
    KoDialog.cpp
    KoGlobal.cpp
    KoZoomWidget.cpp
    KoTagToolButton.cpp
    KoTagChooserWidget.cpp
    KoTagFilterWidget.cpp
    KoResourceTaggingManager.cpp
    KoResourceItemChooserContextMenu.cpp
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
    KoGlobal.cpp
    KoColorPatch.cpp
    KoColorPopupAction.cpp
    KoColorSetWidget.cpp
    KoColorSlider.cpp
    KoEditColorSetDialog.cpp
    KoTriangleColorSelector.cpp
    KoResourcePopupAction.cpp
    KoIconToolTip.cpp
    KoResourceItemChooser.cpp
    KoResourceItemChooserSync.cpp
    KoResourceSelector.cpp
    KoResourceModel.cpp
    KoResourceItemDelegate.cpp
    KoResourceItemView.cpp
    KoResourceTagStore.cpp
    KoRuler.cpp
    #KoRulerController.cpp
    KoItemToolTip.cpp
    KoCheckerBoardPainter.cpp
    KoResourceServerAdapter.cpp
    KoResourceServerProvider.cpp
    KoLineStyleSelector.cpp
    KoLineStyleItemDelegate.cpp
    KoLineStyleModel.cpp
    KoDockWidgetTitleBar.cpp
    KoDockWidgetTitleBarButton.cpp
    KoResourceFiltering.cpp
    KoResourceModelBase.cpp
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
    KoGlobal.cpp
    KoTableView.cpp
    
    WidgetsDebug.cpp

    kis_file_name_requester.cpp

    kis_double_parse_spin_box.cpp
    kis_double_parse_unit_spin_box.cpp
    kis_int_parse_spin_box.cpp

    KisColorSelectorInterface.cpp
    KoAnchorSelectionWidget.cpp

    squeezedcombobox.cpp
)

ki18n_wrap_ui( kritawidgets_LIB_SRCS
    KoConfigAuthorPage.ui
    koDocumentInfoAboutWidget.ui
    koDocumentInfoAuthorWidget.ui
    KoEditColorSet.ui
    wdg_file_name_requester.ui
    KoPageLayoutWidget.ui
    KoShadowConfigWidget.ui
)

add_library(kritawidgets SHARED ${kritawidgets_LIB_SRCS})
generate_export_header(kritawidgets BASE_NAME kritawidgets)
target_link_libraries(kritawidgets kritaodf kritaflake kritapigment kritawidgetutils Qt5::PrintSupport KF5::CoreAddons KF5::ConfigGui KF5::GuiAddons KF5::WidgetsAddons KF5::ConfigCore KF5::Completion)

if(X11_FOUND)
    target_link_libraries(kritawidgets Qt5::X11Extras ${X11_LIBRARIES})
endif()

set_target_properties(kritawidgets PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritawidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})
