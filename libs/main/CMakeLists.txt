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
    KoDockWidgetTitleBar.cpp
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
    KoOdfExporter.cpp
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
)

kde4_add_ui_files( komain_LIB_SRCS
   koDocumentInfoAboutWidget.ui
   koDocumentInfoAuthorWidget.ui
   KoOpenPaneBase.ui
   KoDetailsPaneBase.ui
)

kde4_add_library(komain SHARED ${komain_LIB_SRCS})

target_link_libraries(komain ${extra_libs}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KABC_LIBS}
    ${KDE4_KFILE_LIBRARY}
    kowidgets
    kotext
    koaction
)

target_link_libraries(komain LINK_INTERFACE_LIBRARIES kotext kowidgets ${KDE4_KPARTS_LIBS})

set_target_properties(komain PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS komain ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kodocinfopropspage_PART_SRCS KoDocInfoPropsFactory.cpp )

kde4_add_plugin(kodocinfopropspage ${kodocinfopropspage_PART_SRCS})

target_link_libraries(kodocinfopropspage  ${KDE4_KIO_LIBS} komain )

install(TARGETS kodocinfopropspage DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES kodocinfopropspage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES kofficedocker.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES koffice_shell.rc DESTINATION ${DATA_INSTALL_DIR}/koffice )
install( FILES
    KoApplication.h
    KoApplicationAdaptor.h
    KoDocument.h
    KoDocumentEntry.h
    KoDocumentInfo.h
    KoDocumentInfoDlg.h
    KoEmbeddingFilter.h
    KoFactory.h
    KoFilter.h
    KoFilterChain.h
    KoFilterChainLinkList.h
    KoFilterEntry.h
    KoFilterManager.h
    KoGlobal.h
    KoGridData.h
    KoMainWindow.h
    KoOdfExporter.h
    KoVersionDialog.h
    KoView.h
    KoViewAdaptor.h
    KoImageSelectionWidget.h
    kofficeversion.h
    KoProgressUpdater.h
    KoProgressBar.h
    KoUpdater.h
    KoStandardAction.h
    komain_export.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

