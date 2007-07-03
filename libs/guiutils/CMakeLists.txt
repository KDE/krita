
add_subdirectory( tests )


include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${PIGMENT_INCLUDES}  ${KOGUIUTILS_INCLUDES})


########### next target ###############

set(koguiutils_LIB_SRCS
   KoCharSelectDia.cpp
   KoContextHelp.cpp
   KoCsvImportDialog.cpp
   KoDocumentSectionDelegate.cpp
   KoDocumentSectionView.cpp
   KoDocumentSectionToolTip.cpp
   KoGeneralPropertyUi.cpp
   KoGuideLineDia.cpp 
   KoGuides.cpp 
   KoImageResource.cpp
   KoInsertLink.cpp
   KoItemToolTip.cpp
   #KoPageLayoutColumns.cpp
   #KoPageLayoutDia.cpp
   #KoPageLayoutHeader.cpp
   #KoPageLayoutSize.cpp 
   KoPartSelectDia.cpp
   KoPartSelectAction.cpp
   KoPictureFilePreview.cpp
   KoRuler.cpp
   KoShapeGeometry.cpp
   KoSliderCombo.cpp
   KoTabBar.cpp
   KoTabChooser.cpp
   KoTemplateChooseDia.cpp
   KoTemplateCreateDia.cpp
   KoToolBox.cpp
   KoUnitWidgets.cpp
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
    KoImageData.cpp
    KoImageCollection.cpp
   KoColorDocker.cpp
   KoResourceChooser.cpp
   KoColorSlider.cpp
   KoXYColorSelector.cpp
   KoUniColorChooser.cpp
   KoUniColorDialog.cpp
   KoColorSetWidget.cpp
   KoColorPatch.cpp
   KoDualColorButton.cpp
   KoLineStyleSelector.cpp
)

kde4_automoc(${koguiutils_LIB_SRCS})

kde4_add_ui_files( koguiutils_LIB_SRCS
    KoCsvImportDialog.ui
    KoGeneralPropertyUi.ui
    #KoPageLayoutColumns.ui
    #KoPageLayoutHeaderBase.ui
    KoShapeGeometry.ui
)

kde4_add_library(koguiutils SHARED ${koguiutils_LIB_SRCS})

target_link_libraries(koguiutils  ${KDE4_KDECORE_LIBS} komain flake pigment koresources)

set_target_properties(koguiutils PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS koguiutils  DESTINATION ${LIB_INSTALL_DIR})

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
    KoInsertLink.h
    KoItemToolTip.h
    #KoPageLayoutColumns.h
    #KoPageLayoutDia.h
    #KoPageLayoutHeader.h
    #KoPageLayoutSize.h
    KoPartSelectAction.h
    KoPartSelectDia.h
    KoPictureFilePreview.h
    KoRuler.h
    KoTabBar.h
    KoTabChooser.h
    KoTabulator.h
    KoTemplateChooseDia.h
    KoTemplateCreateDia.h
    KoUnitWidgets.h
    KoZoomAction.h
    KoZoomController.h
    KoZoomHandler.h
    KoZoomMode.h
    KoToolBoxFactory.h
    KoToolDocker.h
    KoToolDockerFactory.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
)
