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
    KoDocInfoPropsFactory.cpp
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
    KoToolBox.cpp
    KoToolBoxFactory.cpp
    KoToolDocker.cpp
    KoVersionDialog.cpp
    KoViewAdaptor.cpp
    KoView.cpp
    kofficeversion.cc
    KoFilterManager.cpp
    KoFilterChain.cpp
    KoFilter.cpp
    KoEmbeddingFilter.cpp
    KoFilterEntry.cpp
    KoFilterManager_p.cpp
    KoFilterVertex.cpp
    KoFilterGraph.cpp
    KoFilterEdge.cpp
    KoFilterChainLink.cpp
    KoFilterChainLinkList.cpp
    KoImageSelectionWidget.cpp
    KoStandardAction.cpp
    config/KoConfigDocumentPage.cpp
    config/KoConfigGridPage.cpp
    config/KoConfigMiscPage.cpp

    KoDocumentSectionDelegate.cpp
    KoDocumentSectionToolTip.cpp
    KoDocumentSectionView.cpp
    KoItemToolTip.cpp

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

    rdf/KoDocumentRdfBase.cpp
    rdf/KoDocumentRdfEditWidgetBase.cpp

    ActionJob_p.cpp
    KoAction.cpp
    KoExecutePolicy.cpp
    KoJobsListPolicy.cpp
)
if( Soprano_FOUND )
    set(komain_LIB_SRCS ${komain_LIB_SRCS}
        rdf/KoDocumentRdf.cpp
        rdf/KoDocumentRdfEditWidget.cpp
        rdf/KoSopranoTableModelDelegate.cpp
        rdf/KoSopranoTableModel.cpp
        rdf/RdfPrefixMapping.cpp
        rdf/SemanticStylesheet.cpp
        rdf/RdfSemanticItem.cpp
        rdf/RdfFoaF.cpp
        rdf/RdfCalendarEvent.cpp
        rdf/RdfLocationEditWidget.cpp
        rdf/RdfLocation.cpp
        rdf/RdfSemanticItemViewSite.cpp
        rdf/RdfSemanticTree.cpp
        rdf/RdfSemanticTreeWidgetItem.cpp
        rdf/RdfSemanticTreeWidgetAction.cpp
        rdf/RdfSemanticTreeWidgetSelectAction.cpp
        rdf/InsertSemanticObjectActionBase.cpp
        rdf/InsertSemanticObjectCreateAction.cpp
        rdf/InsertSemanticObjectReferenceAction.cpp
        rdf/RdfFoaFTreeWidgetItem.cpp
        rdf/RdfCalendarEventTreeWidgetItem.cpp
        rdf/RdfLocationTreeWidgetItem.cpp
        rdf/SemanticStylesheetsEditor.cpp
        rdf/KoChangeTrackerDisabledRAII.cpp
    )
endif( Soprano_FOUND )

kde4_add_ui_files( komain_LIB_SRCS
   koDocumentInfoAboutWidget.ui
   koDocumentInfoAuthorWidget.ui
   KoOpenPaneBase.ui
   KoDetailsPaneBase.ui
)
if( Soprano_FOUND )
    kde4_add_ui_files( komain_LIB_SRCS
       rdf/KoDocumentRdfEditWidget.ui
       rdf/RdfFoaFEditWidget.ui
       rdf/RdfCalendarEventEditWidget.ui
       rdf/RdfLocationEditWidget.ui
       rdf/RdfLocationViewWidget.ui
       rdf/SemanticStylesheetsEditor.ui
    )
endif( Soprano_FOUND )


kde4_add_library(komain SHARED ${komain_LIB_SRCS})

target_link_libraries(komain ${extra_libs}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KABC_LIBS}
    ${KDE4_KFILE_LIBRARY}
    ${KDE4_KCAL_LIBS}
    ${KDE4_THREADWEAVER_LIBRARIES}
    kowidgets
    kotext
)
if( Soprano_FOUND )
    target_link_libraries(komain ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )


target_link_libraries(komain LINK_INTERFACE_LIBRARIES kotext kowidgets ${KDE4_KPARTS_LIBS})

set_target_properties(komain PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS komain ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install(FILES kofficedocker.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES koffice_shell.rc DESTINATION ${DATA_INSTALL_DIR}/koffice )
install( FILES
    KoApplication.h
    KoApplicationAdaptor.h
    KoDocument.h
    KoDocumentEntry.h
    KoDocumentInfo.h
    KoEmbeddingFilter.h
    KoFactory.h
    KoFilter.h
    KoFilterChain.h
    KoFilterChainLinkList.h
    KoFilterManager.h
    KoGlobal.h
    KoGridData.h
    KoMainWindow.h
    KoVersionDialog.h
    KoView.h
    KoViewAdaptor.h
    KoImageSelectionWidget.h
    kofficeversion.h
    KoProgressUpdater.h
    KoProgressBar.h
    KoUpdater.h
    KoStandardAction.h
    rdf/KoDocumentRdfBase.h
    rdf/KoDocumentRdfEditWidgetBase.h
    komain_export.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if( Soprano_FOUND )
    install( FILES
        rdf/RdfForward.h
        rdf/KoDocumentRdf.h
        rdf/KoDocumentRdfEditWidget.h
        rdf/KoSopranoTableModelDelegate.h
        rdf/KoSopranoTableModel.h
        rdf/RdfPrefixMapping.h
        rdf/SemanticStylesheet.h
        rdf/RdfSemanticItem.h
        rdf/RdfFoaF.h
        rdf/RdfCalendarEvent.h
        rdf/RdfLocationEditWidget.h
        rdf/RdfLocation.h
        rdf/RdfSemanticItemViewSite.h
        rdf/RdfSemanticTree.h
        rdf/RdfSemanticTreeWidgetItem.h
        rdf/RdfSemanticTreeWidgetAction.h
        rdf/RdfSemanticTreeWidgetSelectAction.h
        rdf/InsertSemanticObjectActionBase.h
        rdf/InsertSemanticObjectCreateAction.h
        rdf/InsertSemanticObjectReferenceAction.h
        rdf/RdfFoaFTreeWidgetItem.h
        rdf/RdfCalendarEventTreeWidgetItem.h
        rdf/RdfLocationTreeWidgetItem.h
        rdf/SemanticStylesheetsEditor.h
        rdf/KoChangeTrackerDisabledRAII.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif( Soprano_FOUND )

