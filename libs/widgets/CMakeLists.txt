add_subdirectory( tests )
add_subdirectory( pics )

include_directories(${KOTEXT_INCLUDES} ${PIGMENT_INCLUDES})

set(kowidgets_LIB_SRCS
    KoAspectButton.cpp
    KoCsvImportDialog.cpp
    KoImageResource.cpp
    KoPageLayout.cpp
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
    KoShadowConfigWidget.cpp
    KoIconToolTip.cpp
    KoResourceItemChooser.cpp
    KoResourceSelector.cpp
    KoResourceModel.cpp
    KoResourceItemDelegate.cpp
    KoResourceItemView.cpp
    KoItemToolTip.cpp
    KoCheckerBoardPainter.cpp
    KoResourceServerAdapter.cpp
    KoResourceServerProvider.cpp

)

kde4_add_ui_files( kowidgets_LIB_SRCS
    KoCsvImportDialog.ui
    KoPageLayoutWidget.ui
    KoEditColorSet.ui
    KoShadowConfigWidget.ui
)

kde4_add_library(kowidgets SHARED ${kowidgets_LIB_SRCS})

target_link_libraries(kowidgets kotext pigmentcms)

target_link_libraries(kowidgets LINK_INTERFACE_LIBRARIES pigmentcms ${KDE4_KDEUI_LIBS})

set_target_properties(kowidgets PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

install(TARGETS kowidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    KoGenericRegistryModel.h
    KoPageLayoutDialog.h
    KoPageLayout.h
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

    DESTINATION
    ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
