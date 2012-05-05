add_subdirectory( tests )

if (KdepimLibs_FOUND)
   include_directories(${KDEPIMLIBS_INCLUDE_DIR})
   add_definitions(-DKDEPIMLIBS_FOUND)
endif (KdepimLibs_FOUND)

include_directories( ${KOMAIN_INCLUDES})

########### next target ###############

if( QCA2_FOUND )
   add_definitions( -DQCA2 )
endif( QCA2_FOUND )

set(komain_LIB_SRCS
    KoApplicationAdaptor.cpp
    KoApplication.cpp
    KoDockerManager.cpp
    KoDocumentAdaptor.cpp
    KoDocument.cpp
    KoDocumentEntry.cpp
    KoDocumentInfo.cpp
    KoDocumentInfoDlg.cpp
    KoDocumentInfoPropsPage.cpp
    KoFactory.cpp
    KoFileDialog.cpp
    KoGlobal.cpp
    KoGridData.cpp
    KoMainWindow.cpp
    KoPrintingDialog.cpp
    KoPrintJob.cpp
    KoRulerController.cpp
    KoRuler.cpp
    KoModeBox.cpp
    KoModeBoxDocker.cpp
    KoModeBoxFactory.cpp
    KoToolBox.cpp
    KoToolBoxDocker.cpp
    KoToolBoxFactory.cpp
    KoToolDocker.cpp
    KoVersionDialog.cpp
    KoViewAdaptor.cpp
    KoView.cpp
    calligraversion.cpp
    KoFilterManager.cpp
    KoFilterChain.cpp
    KoFilter.cpp
    KoFilterEntry.cpp
    KoFilterManager_p.cpp
    KoFilterVertex.cpp
    KoFilterGraph.cpp
    KoFilterEdge.cpp
    KoFilterChainLink.cpp
    KoFilterChainLinkList.cpp
    KoStandardAction.cpp
    KoUndoStackAction.cpp
    config/KoConfigDocumentPage.cpp
    config/KoConfigGridPage.cpp
    config/KoConfigMiscPage.cpp

    KoDocumentSectionDelegate.cpp
    KoDocumentSectionToolTip.cpp
    KoDocumentSectionView.cpp

    KoProgressBar.cpp
    KoProgressUpdater.cpp
    KoUpdater.cpp
    KoUpdaterPrivate_p.cpp

    KoDetailsPane.cpp
    KoExistingDocumentPane.cpp
    KoOpenPane.cpp
    KoRecentDocumentsPane.cpp
    KoTemplate.cpp
    KoTemplateCreateDia.cpp
    KoTemplateGroup.cpp
    KoTemplates.cpp
    KoTemplatesPane.cpp
    KoTemplateTree.cpp

    rdf/KoDocumentRdfEditWidgetBase.cpp

    KoFindBase.cpp
    KoFindMatch.cpp
    KoFindText.cpp
    KoFindToolbar.cpp
    KoFindOption.cpp
    KoFindOptionSet.cpp
)
if( Soprano_FOUND )
    set(komain_LIB_SRCS ${komain_LIB_SRCS}
        rdf/KoDocumentRdf.cpp
        rdf/KoDocumentRdfEditWidget.cpp
        rdf/KoSopranoTableModelDelegate.cpp
        rdf/KoSopranoTableModel.cpp
        rdf/KoRdfPrefixMapping.cpp
        rdf/KoSemanticStylesheet.cpp
        rdf/KoRdfSemanticItem.cpp
        rdf/KoRdfFoaF.cpp
        rdf/KoRdfCalendarEvent.cpp
        rdf/KoRdfLocationEditWidget.cpp
        rdf/KoRdfLocation.cpp
        rdf/KoRdfSemanticItemViewSite.cpp
        rdf/KoRdfSemanticTree.cpp
        rdf/KoRdfSemanticTreeWidgetItem.cpp
        rdf/KoRdfFoaFTreeWidgetItem.cpp
        rdf/KoRdfCalendarEventTreeWidgetItem.cpp
        rdf/KoRdfLocationTreeWidgetItem.cpp
        rdf/KoSemanticStylesheetsEditor.cpp
        rdf/KoChangeTrackerDisabledRAII.cpp
        rdf/RdfSemanticTreeWidgetAction.cpp
        rdf/RdfSemanticTreeWidgetSelectAction.cpp
        rdf/InsertSemanticObjectActionBase.cpp
        rdf/InsertSemanticObjectCreateAction.cpp
        rdf/InsertSemanticObjectReferenceAction.cpp
    )
endif( Soprano_FOUND )

kde4_add_ui_files( komain_LIB_SRCS
   koDocumentInfoAboutWidget.ui
   koDocumentInfoAuthorWidget.ui
   KoOpenPaneBase.ui
   KoDetailsPaneBase.ui
)
if(Soprano_FOUND)

    kde4_add_ui_files( komain_LIB_SRCS
       rdf/KoDocumentRdfEditWidget.ui
       rdf/KoRdfFoaFEditWidget.ui
       rdf/KoRdfCalendarEventEditWidget.ui
       rdf/KoSemanticStylesheetsEditor.ui
    )

#    if(MARBLE_FOUND)
#        kde4_add_ui_files( komain_LIB_SRCS
#           rdf/KoRdfLocationEditWidgetMarble.ui
#           rdf/KoRdfLocationViewWidgetMarble.ui
#        )
#    else(MARBLE_FOUND)
        kde4_add_ui_files( komain_LIB_SRCS
           rdf/KoRdfLocationEditWidget.ui
           rdf/KoRdfLocationViewWidget.ui
        )
#    endif(MARBLE_FOUND)

endif(Soprano_FOUND)


kde4_add_library(komain SHARED ${komain_LIB_SRCS})

target_link_libraries(komain ${extra_libs}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KABC_LIBS}
    ${KDE4_KFILE_LIBRARY}
    ${KDE4_KCAL_LIBS}
    kowidgets
    textlayout
)
if( Soprano_FOUND )
    target_link_libraries(komain ${SOPRANO_LIBRARIES})
    target_link_libraries(komain LINK_INTERFACE_LIBRARIES  ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )
#if( MARBLE_FOUND )
#    include_directories(${MARBLE_INCLUDE_DIR})
#    target_link_libraries(komain ${MARBLE_LIBRARIES})
#    target_link_libraries(komain LINK_INTERFACE_LIBRARIES  ${MARBLE_LIBRARIES})
#endif( MARBLE_FOUND )

target_link_libraries(komain LINK_INTERFACE_LIBRARIES kotext kowidgets ${KDE4_KPARTS_LIBS})

set_target_properties(komain PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS komain ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### plugin for the properties dialog ###############

set(kodocinfopropspage_PART_SRCS KoDocInfoPropsFactory.cpp )

kde4_add_plugin(kodocinfopropspage ${kodocinfopropspage_PART_SRCS})

target_link_libraries(kodocinfopropspage  ${KDE4_KIO_LIBS} komain )

install(TARGETS kodocinfopropspage DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install(FILES kodocinfopropspage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES calligradocker.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES calligra_shell.rc DESTINATION ${DATA_INSTALL_DIR}/calligra )
install( FILES
    KoApplication.h
    KoApplicationAdaptor.h
    KoDocument.h
    KoDocumentEntry.h
    KoDocumentInfo.h
    KoDocumentSectionModel.h
    KoFactory.h
    KoFilter.h
    KoFilterChain.h
    KoFilterChainLinkList.h
    KoFilterEntry.h
    KoFilterGraph.h
    KoFilterManager.h
    KoFilterVertex.h
    KoFindBase.h
    KoFindText.h
    KoFindToolbar.h
    KoFindMatch.h
    KoFindOption.h
    KoFindOptionSet.h
    KoGlobal.h
    KoGridData.h
    KoMainWindow.h
    KoModeBoxFactory.h
    KoPrintJob.h
    KoProgressUpdater.h
    KoProgressProxy.h
    KoVersionDialog.h
    KoView.h
    KoViewAdaptor.h
    calligraversion.h
    KoProgressUpdater.h
    KoProgressBar.h
    KoUpdater.h
    KoStandardAction.h
    komain_export.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if( Soprano_FOUND )
    install( FILES
        rdf/KoRdfFoaF.h
        rdf/KoRdfCalendarEvent.h
        rdf/KoRdfLocation.h
        rdf/KoRdfFoaFTreeWidgetItem.h
        rdf/KoRdfCalendarEventTreeWidgetItem.h
        rdf/KoRdfLocationTreeWidgetItem.h
        rdf/KoChangeTrackerDisabledRAII.h
        rdf/RdfForward.h
        rdf/KoDocumentRdf.h
        rdf/KoSopranoTableModelDelegate.h
        rdf/KoSopranoTableModel.h
        rdf/KoSemanticStylesheet.h
        rdf/KoRdfSemanticItem.h
        rdf/KoRdfSemanticItemViewSite.h
        rdf/KoRdfSemanticTree.h
        rdf/KoRdfSemanticTreeWidgetItem.h
        rdf/RdfSemanticTreeWidgetAction.h
        rdf/RdfSemanticTreeWidgetSelectAction.h
        rdf/InsertSemanticObjectActionBase.h
        rdf/InsertSemanticObjectCreateAction.h
        rdf/InsertSemanticObjectReferenceAction.h
        rdf/KoSemanticStylesheetsEditor.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif( Soprano_FOUND )

