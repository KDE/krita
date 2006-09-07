

add_subdirectory( pics )


include_directories( ${KOFFICECORE_INCLUDES} ${FLAKE_INCLUDES} )


########### next target ###############

set(kofficeui_LIB_SRCS
   KoBrush.cpp 
   KoCharSelectDia.cpp
   KoContextHelp.cpp
   KoCsvImportDialog.cpp
   KoDocumentSectionDelegate.cpp
   KoDocumentSectionView.cpp
   KoDocumentSectionToolTip.cpp
   KoEditPath.cpp 
   KoGeneralPropertyUi.cpp
   KoGuideLineDia.cpp 
   KoGuides.cpp 
   KoImageResource.cpp
   KoInsertLink.cpp
   KoItemToolTip.cpp
   KoKoolBar.cpp
   KoPageLayoutColumns.cpp
   KoPageLayoutDia.cpp
   KoPageLayoutHeader.cpp
   KoPageLayoutSize.cpp 
   KoPartSelectDia.cpp
   KoPartSelectAction.cpp
   KoPen.cpp
   KoPictureFilePreview.cpp
   KoRuler.cpp
   KoShapeSelector.cpp
   KoShapeGeometry.cpp
   KoTabBar.cpp
   KoTabChooser.cpp
   KoTemplateChooseDia.cpp
   KoTemplateCreateDia.cpp
   KoToolBox.cpp
   KoToolManager.cpp
   KoToolManager_p.cpp
   KoToolRegistry.cpp
   KoUnitWidgets.cpp
   KoZoomAction.cpp
   KoZoomHandler.cpp
   KoZoomMode.cpp
   tkaction.cpp
   tkcoloractions.cpp 
   tkcombobox.cpp 
   tktoolbarbutton.cpp 
)

kde4_automoc(${kofficeui_LIB_SRCS})

kde4_add_ui_files( kofficeui_LIB_SRCS
    KoCsvImportDialog.ui
    KoGeneralPropertyUi.ui
    KoPageLayoutColumns.ui
    KoPageLayoutHeaderBase.ui
    KoShapeGeometry.ui
)

kde4_add_library(kofficeui SHARED ${kofficeui_LIB_SRCS})

target_link_libraries(kofficeui  ${KDE4_KDECORE_LIBS} kofficecore flake )

set_target_properties(kofficeui PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kofficeui  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############
install(
    FILES
    KoBrush.h
    KoCharSelectDia.h
    KoContextHelp.h
    KoDocumentSectionModel.h
    KoDocumentSectionView.h
    KoEditPath.h
    KoGeneralPropertyUi.h
    KoGuideLineDia.h
    KoGuides.h
    KoImageResource.h
    KoInsertLink.h
    KoItemToolTip.h
    KoKoolBar.h
    KoPageLayoutColumns.h
    KoPageLayoutDia.h
    KoPageLayoutHeader.h
    KoPageLayoutSize.h
    KoPartSelectAction.h
    KoPartSelectDia.h
    KoPen.h
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
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
)
