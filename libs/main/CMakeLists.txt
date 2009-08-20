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
    KoToolDockerFactory.cpp
    KoUndoStack.cpp
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
    config/KoConfigDocumentPage.cpp
    config/KoConfigGridPage.cpp
)

if (NOT NO_PIGMENT)
    set(komain_LIB_SRCS ${komain_LIB_SRCS}
        KoColorDocker.cpp
    )
endif (NOT NO_PIGMENT)

kde4_add_ui_files( komain_LIB_SRCS
   koDocumentInfoAboutWidget.ui
   koDocumentInfoAuthorWidget.ui
)

kde4_add_library(komain SHARED ${komain_LIB_SRCS})

if (NOT NO_PIGMENT)
    set (extra_libs kocolorwidgets koresources pigmentcms koresourcewidget )
endif (NOT NO_PIGMENT)

target_link_libraries(komain ${extra_libs}
                      flake koaction koaspectbutton kocharacterselectiondialog kocvsimport
                      kodocumentsectionbox koinput kolinestyleselector koopenpane koprogress
                      kozoomaction kotext kopagelayout
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KFILE_LIBRARY}
    ${KDE4_KABC_LIBS}
    ${KDE4_THREADWEAVER_LIBRARIES}
)

target_link_libraries(komain LINK_INTERFACE_LIBRARIES ${extra_libs}
                      flake koaction koaspectbutton kobase kocharacterselectiondialog kocvsimport
                      kodocumentsectionbox koinput kolinestyleselector koodf koopenpane kopicture koplugin koprogress
                      kostore kozoomaction kotext kopagelayout
                      ${QT_QTGUI_LIBRARY})

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
    KoUndoStack.h 
    komain_export.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

