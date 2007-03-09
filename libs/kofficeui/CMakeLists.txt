
add_subdirectory( pics )
add_subdirectory( tests )


include_directories( ${KOFFICECORE_INCLUDES} ${FLAKE_INCLUDES} )


########### next target ###############

set(kofficeui_LIB_SRCS
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
   KoShapeSelector.cpp
   KoShapeGeometry.cpp
   KoTabBar.cpp
   KoTabChooser.cpp
   KoTemplateChooseDia.cpp
   KoTemplateCreateDia.cpp
   KoToolBox.cpp
   KoUnitWidgets.cpp
   KoZoomAction.cpp
   KoZoomHandler.cpp
   KoZoomMode.cpp
   tkaction.cpp
   tkcoloractions.cpp 
   tkcombobox.cpp 
   tktoolbarbutton.cpp
   KoToolBoxFactory.cpp
   KoShapeSelectorFactory.cpp
    KoToolDocker.cpp
    KoToolDockerFactory.cpp
)

kde4_automoc(${kofficeui_LIB_SRCS})

kde4_add_ui_files( kofficeui_LIB_SRCS
    KoCsvImportDialog.ui
    KoGeneralPropertyUi.ui
    #KoPageLayoutColumns.ui
    #KoPageLayoutHeaderBase.ui
    KoShapeGeometry.ui
)

kde4_add_library(kofficeui SHARED ${kofficeui_LIB_SRCS})

target_link_libraries(kofficeui  ${KDE4_KDECORE_LIBS} kofficecore flake )

set_target_properties(kofficeui PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kofficeui  DESTINATION ${LIB_INSTALL_DIR})

install( FILES zoom-width.png zoom-page.png zoom-pixels.png DESTINATION ${DATA_INSTALL_DIR}/koffice/icons/crystalsvg/22x22/actions)

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
    KoZoomHandler.h
    KoZoomMode.h
    tkaction.h
    tkcoloractions.h
    tkcombobox.h
    tktoolbarbutton.h
    KoToolBoxFactory.h
    KoShapeSelectorFactory.h
    KoToolDocker.h
    KoToolDockerFactory.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
)
