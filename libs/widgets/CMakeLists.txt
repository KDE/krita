add_subdirectory( tests )

include_directories(${KOWIDGETS_INCLUDES})
include_directories(${CMAKE_SOURCE_DIR}/libs/widgetutils)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
    KoResourceFiltering.cpp
    KoResourceModelBase.cpp

    KoToolBoxButton.cpp
    KoToolBox.cpp
    KoToolBoxDocker.cpp
    KoToolBoxFactory.cpp
    KoToolDocker.cpp
    
    KoDocumentInfoDlg.cpp
    KoGlobal.cpp
    KoTableView.cpp
    
    WidgetsDebug.cpp

    kis_url_requester.cpp
)

ki18n_wrap_ui( kritawidgets_LIB_SRCS
    KoConfigAuthorPage.ui
    koDocumentInfoAboutWidget.ui
    koDocumentInfoAuthorWidget.ui
    KoEditColorSet.ui
    KoPageLayoutWidget.ui
    KoShadowConfigWidget.ui
    wdg_url_requester.ui
)

qt5_add_resources(kritawidgets_LIB_SRCS kritawidgets.qrc)

add_library(kritawidgets SHARED ${kritawidgets_LIB_SRCS})
generate_export_header(kritawidgets BASE_NAME kritawidgets)
target_link_libraries(kritawidgets kritatext kritapigment kritawidgetutils KF5::GuiAddons KF5::WidgetsAddons KF5::ConfigCore KF5::Completion)

if(X11_FOUND)
    target_link_libraries(kritawidgets Qt5::X11Extras ${X11_LIBRARIES})
endif()

if (APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
    target_link_libraries(kritawidgets ${FOUNDATION_LIBRARY})
endif ()

target_link_libraries(kritawidgets LINK_INTERFACE_LIBRARIES kritatext kritapigment kritawidgetutils)

set_target_properties(kritawidgets PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kritawidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})
