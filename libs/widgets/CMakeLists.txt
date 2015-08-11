add_subdirectory( tests )
add_subdirectory( pics )

include_directories(${KOTEXT_INCLUDES} ${KOODF_INCLUDES} ${PIGMENT_INCLUDES})
include_directories(${CMAKE_SOURCE_DIR}/libs/widgetutils)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
if (LIBATTICA_FOUND)
    include_directories(${LIBATTICA_INCLUDE_DIR})
endif ()

set(kowidgets_LIB_SRCS
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

ki18n_wrap_ui( kowidgets_LIB_SRCS
    KoConfigAuthorPage.ui
    KoCsvImportDialog.ui
    koDocumentInfoAboutWidget.ui
    koDocumentInfoAuthorWidget.ui
    KoEditColorSet.ui
    KoPageLayoutWidget.ui
    KoShadowConfigWidget.ui
)


add_library(kowidgets SHARED ${kowidgets_LIB_SRCS})
generate_export_header(kowidgets BASE_NAME kowidgets)

target_link_libraries(kowidgets kotext pigmentcms kowidgetutils KF5::GuiAddons KF5::WidgetsAddons KF5::ConfigCore KF5::Codecs KF5::IconThemes )

if(GHNS)
    target_link_libraries(kowidgets KF5::NewStuff)
endif ()

if(X11_FOUND)
    target_link_libraries(kowidgets Qt5::X11Extras ${X11_LIBRARIES})
endif()


target_link_libraries(kowidgets LINK_INTERFACE_LIBRARIES kotext pigmentcms kowidgetutils)

set_target_properties(kowidgets PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kowidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    KoGlobal.h
    KoResourceItemChooserContextMenu.h
    KoGenericRegistryModel.h
    KoPageLayoutDialog.h
    KoPageLayoutWidget.h
    KoPagePreviewWidget.h
    KoPositionSelector.h
    ${CMAKE_CURRENT_BINARY_DIR}/kowidgets_export.h
    KoZoomAction.h
    KoZoomController.h
    KoZoomInput.h
    KoDpi.h
    KoZoomHandler.h
    KoZoomMode.h
    KoGlobal.h
    KoColorPatch.h
    KoStrokeConfigWidget.h
    KoFillConfigWidget.h
    KoShadowConfigWidget.h
    KoColorPopupAction.h
    KoColorSetWidget.h
    KoColorSlider.h
    KoDualColorButton.h
    KoEditColorSetDialog.h
    KoTriangleColorSelector.h
    KoResourceItemChooser.h
    KoResourceSelector.h
    KoResourceServer.h
    KoResourceServerAdapter.h
    KoResourceServerObserver.h
    KoResourceServerProvider.h
    KoResourceTagStore.h
    KoLineStyleSelector.h
    KoDockWidgetTitleBar.h
    KoDockWidgetTitleBarButton.h

    KoResourceModelBase.h

    KoGlobal.h

    DESTINATION
    ${INCLUDE_INSTALL_DIR}/calligra
COMPONENT Devel)
