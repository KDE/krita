add_subdirectory( tests )
add_subdirectory( pics )

include_directories(${KOTEXT_INCLUDES} ${KOODF_INCLUDES} ${PIGMENT_INCLUDES})
include_directories(${CMAKE_SOURCE_DIR}/libs/widgetutils)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
if (LIBATTICA_FOUND)
    include_directories(${LIBATTICA_INCLUDE_DIR})
endif ()

set(kritawidgets_LIB_SRCS
    KoGradientEditWidget.cpp
    KoResourcePaths.cpp
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
    KoCsvImportDialog.cpp
    KoPageLayoutDialog.cpp
    KoPageLayoutWidget.cpp
    KoPagePreviewWidget.cpp
    KoPositionSelector.cpp
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
    KoDualColorButton.cpp
    KoEditColorSetDialog.cpp
    KoTriangleColorSelector.cpp
    KoResourcePopupAction.cpp
    KoStrokeConfigWidget.cpp
    KoFillConfigWidget.cpp
    KoShadowConfigWidget.cpp
    KoIconToolTip.cpp
    KoResourceItemChooser.cpp
    KoResourceItemChooserSync.cpp
    KoResourceSelector.cpp
    KoResourceModel.cpp
    KoResourceItemDelegate.cpp
    KoResourceItemView.cpp
    KoResourceTagStore.cpp
    KoRuler.cpp
    KoRulerController.cpp
    KoItemToolTip.cpp
    KoCheckerBoardPainter.cpp
    KoResourceServerAdapter.cpp
    KoResourceServerProvider.cpp
    KoLineStyleSelector.cpp
    KoLineStyleItemDelegate.cpp
    KoLineStyleModel.cpp
    KoMarkerModel.cpp
    KoMarkerItemDelegate.cpp
    KoMarkerSelector.cpp
    KoDockWidgetTitleBar.cpp
    KoDockWidgetTitleBarButton.cpp
    KoViewItemContextBar.cpp
    KoContextBarButton.cpp
    KoResourceFiltering.cpp
    KoResourceModelBase.cpp

    KoToolBoxButton.cpp
    KoToolBox.cpp
    KoToolBoxDocker.cpp
    KoToolBoxFactory.cpp
    KoToolDocker.cpp
    
    KoModeBox.cpp
    KoModeBoxDocker.cpp
    KoModeBoxFactory.cpp

    KoDocumentInfoDlg.cpp
    KoDocumentInfoPropsPage.cpp

    KoGlobal.cpp
    KoTableView.cpp
    
    KoIconUtils.cpp

    WidgetsDebug.cpp

)

ki18n_wrap_ui( kritawidgets_LIB_SRCS
    KoConfigAuthorPage.ui
    KoCsvImportDialog.ui
    koDocumentInfoAboutWidget.ui
    koDocumentInfoAuthorWidget.ui
    KoEditColorSet.ui
    KoPageLayoutWidget.ui
    KoShadowConfigWidget.ui
)


add_library(kritawidgets SHARED ${kritawidgets_LIB_SRCS})
generate_export_header(kritawidgets BASE_NAME kritawidgets)
target_link_libraries(kritawidgets kritatext kritapigment kritawidgetutils KF5::ConfigWidgets KF5::GuiAddons KF5::WidgetsAddons KF5::ConfigCore KF5::Codecs KF5::IconThemes KF5::Completion)

if(GHNS)
    target_link_libraries(kritawidgets KF5::NewStuff)
endif ()

if(X11_FOUND)
    target_link_libraries(kritawidgets Qt5::X11Extras ${X11_LIBRARIES})
endif()


target_link_libraries(kritawidgets LINK_INTERFACE_LIBRARIES kritatext kritapigment kritawidgetutils)

set_target_properties(kritawidgets PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritawidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})
