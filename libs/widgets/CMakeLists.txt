add_subdirectory( tests )
add_subdirectory( pics )

include_directories(${KOTEXT_INCLUDES} ${KOODF_INCLUDES} ${PIGMENT_INCLUDES})
if (LIBATTICA_FOUND)
    include_directories(${LIBATTICA_INCLUDE_DIR})
endif (LIBATTICA_FOUND)

if(NEPOMUK)
    include(SopranoAddOntology)
    include_directories(${SOPRANO_INCLUDE_DIR} ${NEPOMUK_INCLUDES})
endif(NEPOMUK)

set(kowidgets_LIB_SRCS
    KoAspectButton.cpp
    KoCsvImportDialog.cpp
    KoImageResource.cpp
    KoPageLayoutDialog.cpp
    KoPageLayoutWidget.cpp
    KoPagePreviewWidget.cpp
    KoPositionSelector.cpp
    KoSliderCombo.cpp
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
    KoDualColorButton.cpp
    KoEditColorSetDialog.cpp
    KoTriangleColorSelector.cpp
    KoUniColorChooser.cpp
    KoUniColorDialog.cpp
    KoXYColorSelector.cpp
    KoStrokeConfigWidget.cpp
    KoShadowConfigWidget.cpp
    KoIconToolTip.cpp
    KoResourceItemChooser.cpp
    KoResourceSelector.cpp
    KoResourceModel.cpp
    KoResourceItemDelegate.cpp
    KoResourceItemView.cpp
    KoResourceTagging.cpp
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
)

kde4_add_ui_files( kowidgets_LIB_SRCS
    KoCsvImportDialog.ui
    KoPageLayoutWidget.ui
    KoEditColorSet.ui
    KoShadowConfigWidget.ui
)


kde4_add_library(kowidgets SHARED ${kowidgets_LIB_SRCS})

if(GHNS) 
    target_link_libraries(kowidgets kotext pigmentcms ${KDE4_KNEWSTUFF3_LIBS})
else(GHNS)
    target_link_libraries(kowidgets kotext pigmentcms )
endif (GHNS)

if(NEPOMUK)
    target_link_libraries(kowidgets kotext pigmentcms ${NEPOMUK_LIBRARIES} ${SOPRANO_LIBRARIES})
endif(NEPOMUK)

target_link_libraries(kowidgets LINK_INTERFACE_LIBRARIES kotext pigmentcms ${KDE4_KDEUI_LIBS})

set_target_properties(kowidgets PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kowidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    KoGenericRegistryModel.h
    KoPageLayoutDialog.h
    KoPageLayoutWidget.h
    KoPagePreviewWidget.h
    KoPositionSelector.h
    kowidgets_export.h
    KoZoomAction.h
    KoZoomController.h
    KoZoomInput.h
    KoDpi.h
    KoZoomHandler.h
    KoZoomMode.h

    KoColorPatch.h
    KoStrokeConfigWidget.h
    KoShadowConfigWidget.h
    KoColorPopupAction.h
    KoColorSetWidget.h
    KoColorSlider.h
    KoDualColorButton.h
    KoEditColorSetDialog.h
    KoTriangleColorSelector.h
    KoUniColorChooser.h
    KoUniColorDialog.h
    KoXYColorSelector.h
    KoResourceItemChooser.h
    KoResourceSelector.h
    KoResourceTagging.h
    KoLineStyleSelector.h
    KoDockWidgetTitleBar.h
    KoDockWidgetTitleBar.h
    KoDockWidgetTitleBarButton.h

    DESTINATION
    ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
