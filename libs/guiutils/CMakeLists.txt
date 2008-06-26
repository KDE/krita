
add_subdirectory( tests )


include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${PIGMENT_INCLUDES}  ${KOGUIUTILS_INCLUDES} ${KORESOURCES_INCLUDES} ${KOTEXT_INCLUDES} )


########### next target ###############

set(koguiutils_LIB_SRCS
    KoAspectButton.cpp
   KoCharSelectDia.cpp
   KoContextHelp.cpp
   KoCsvImportDialog.cpp
   KoDockerManager.cpp
   KoDocumentSectionDelegate.cpp
   KoDocumentSectionView.cpp
   KoDocumentSectionToolTip.cpp
   KoGeneralPropertyUi.cpp
   KoGuideLineDia.cpp 
   KoGuides.cpp 
   KoImageResource.cpp
   KoItemToolTip.cpp
   KoPartSelectDia.cpp
   KoPartSelectAction.cpp
   KoRuler.cpp
   KoSliderCombo.cpp
   KoTabBar.cpp
   KoTemplateCreateDia.cpp
   KoToolBox.cpp
    KoUnitDoubleSpinBox.cpp
   KoZoomAction.cpp
   KoZoomController.cpp
   KoZoomHandler.cpp
   KoZoomMode.cpp
   KoToolBoxFactory.cpp
    KoToolDocker.cpp
    KoToolDockerFactory.cpp
    KoLayoutVisitor.cpp
    KoCopyController.cpp
    KoPasteController.cpp
   KoColorDocker.cpp
   KoResourceChooser.cpp
   KoColorSlider.cpp
   KoXYColorSelector.cpp
   KoUniColorChooser.cpp
   KoUniColorDialog.cpp
   KoColorSetWidget.cpp
   KoColorSetAction.cpp
   KoEditColorSetDialog.cpp
   KoColorPatch.cpp
   KoDualColorButton.cpp
   KoLineStyleSelector.cpp
    KoProgressUpdater.cpp
    KoPrintingDialog.cpp
   KoShapePainter.cpp
   KoProgressBar.cpp
   KoPositionSelector.cpp
   KoResourceItemChooser.cpp
    KoCheckerBoardPainter.cpp
   KoSelectShapeAction.cpp
   KoZoomInput.cpp
    KoShadowConfigWidget.cpp
    KoPageLayoutDialog.cpp
    KoPageLayoutWidget.cpp
    KoPagePreviewWidget.cpp
)

kde4_add_ui_files( koguiutils_LIB_SRCS
    KoCsvImportDialog.ui
    KoGeneralPropertyUi.ui
    KoPageLayoutWidget.ui
    #KoPageLayoutColumns.ui
    #KoPageLayoutHeaderBase.ui
    KoEditColorSet.ui
    KoShadowConfigWidget.ui
)

kde4_add_library(koguiutils SHARED ${koguiutils_LIB_SRCS})

target_link_libraries(koguiutils  ${KDE4_KDECORE_LIBS} komain flake pigmentcms koresources)

set_target_properties(koguiutils PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koguiutils  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES zoom-width.png zoom-page.png zoom-pixels.png DESTINATION ${DATA_INSTALL_DIR}/koffice/icons/hicolor/22x22/actions)

########### install files ###############
install(
    FILES
    KoCharSelectDia.h
    KoContextHelp.h
    KoDocumentSectionModel.h
    KoDocumentSectionView.h
    KoGeneralPropertyUi.h
    KoGuideLineDia.h
    KoGuides.h
    KoImageResource.h
    KoItemToolTip.h
    KoPartSelectAction.h
    KoPartSelectDia.h
    KoRuler.h
    KoShapePainter.h
    KoTabBar.h
#    KoTabulator.h
    KoTemplateCreateDia.h
    KoToolBox.h
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
