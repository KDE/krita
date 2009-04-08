
add_subdirectory( tests )


include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${PIGMENT_INCLUDES}  ${KOGUIUTILS_INCLUDES} ${KORESOURCES_INCLUDES} ${KOTEXT_INCLUDES} )

add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(koguiutils_LIB_SRCS
   KoAspectButton.cpp
   KoCharSelectDia.cpp
   KoCheckerBoardPainter.cpp
   KoColorDocker.cpp
   KoColorPatch.cpp
   KoColorPopupAction.cpp
   KoColorSetWidget.cpp
   KoColorSlider.cpp
   KoContextHelp.cpp
   KoCopyController.cpp
   KoCsvImportDialog.cpp
   KoCutController.cpp
   KoDockerManager.cpp
   KoDocumentSectionDelegate.cpp
   KoDocumentSectionToolTip.cpp
   KoDocumentSectionView.cpp
   KoDualColorButton.cpp
   KoEditColorSetDialog.cpp
   KoGeneralPropertyUi.cpp
   KoGuideLineDia.cpp 
   KoGuides.cpp 
   KoImageResource.cpp
   KoItemToolTip.cpp
   KoLayoutVisitor.cpp
   KoLineStyleSelector.cpp
   KoPageLayoutDialog.cpp
   KoPageLayoutWidget.cpp
   KoPagePreviewWidget.cpp
   KoPasteController.cpp
   KoPositionSelector.cpp
   KoPrintingDialog.cpp
   KoProgressBar.cpp
   KoProgressUpdater.cpp
   KoUpdaterPrivate.cpp
   KoResourceChooser.cpp
   KoResourceItemChooser.cpp
   KoResourceSelector.cpp
   KoRuler.cpp
   KoSelectShapeAction.cpp
   KoShadowConfigWidget.cpp
   KoShapePainter.cpp
   KoSliderCombo.cpp
   KoTabBar.cpp
   KoTemplateCreateDia.cpp
   KoToolBox.cpp
   KoToolBoxFactory.cpp
   KoToolDocker.cpp
   KoToolDockerFactory.cpp
   KoTriangleColorSelector.cpp
   KoUniColorChooser.cpp
   KoUniColorDialog.cpp
   KoUnitDoubleSpinBox.cpp
   KoUpdater.cpp
   KoXYColorSelector.cpp
   KoZoomAction.cpp
   KoZoomController.cpp
   KoZoomHandler.cpp
   KoZoomInput.cpp
   KoZoomMode.cpp
)

kde4_add_ui_files( koguiutils_LIB_SRCS
    KoCsvImportDialog.ui
    KoGeneralPropertyUi.ui
    KoPageLayoutWidget.ui
    KoEditColorSet.ui
    KoShadowConfigWidget.ui
)

kde4_add_library(koguiutils SHARED ${koguiutils_LIB_SRCS})

target_link_libraries(koguiutils ${KDE4_KDE3SUPPORT_LIBS} flake koresources)
target_link_libraries(koguiutils LINK_INTERFACE_LIBRARIES flake koresources)

set_target_properties(koguiutils PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koguiutils  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES zoom-width.png zoom-page.png zoom-pixels.png DESTINATION ${DATA_INSTALL_DIR}/koffice/icons/hicolor/22x22/actions)

########### install files ###############
install(
    FILES
    koguiutils_export.h
    KoCharSelectDia.h
    KoContextHelp.h
    KoDocumentSectionModel.h
    KoDocumentSectionView.h
    KoGeneralPropertyUi.h
    KoGuideLineDia.h
    KoGuides.h
    KoImageResource.h
    KoItemToolTip.h
    KoProgressUpdater.h
    KoRuler.h
    KoShapePainter.h
    KoTabBar.h
#    KoTabulator.h
    KoTemplateCreateDia.h
    KoToolBoxFactory.h
    KoToolDocker.h
    KoToolDockerFactory.h
    KoUnitDoubleSpinBox.h
    KoZoomAction.h
    KoZoomController.h
    KoZoomHandler.h
    KoZoomMode.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
COMPONENT Devel)
