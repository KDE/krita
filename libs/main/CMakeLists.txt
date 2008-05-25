

add_subdirectory( tests )
add_subdirectory( threadAction/tests )

include_directories( ${KOODF_INCLUDES} ${KDE4_INCLUDE_DIR}/threadweaver/)

########### next target ###############

if( QCA2_FOUND )
add_definitions( -DQCA2 )
endif( QCA2_FOUND )

set(komain_LIB_SRCS
   KoDocument.cpp
   KoPluginLoader.cpp
   KoDocumentAdaptor.cpp
   KoProperties.cpp
   KoGlobal.cpp
   KoFilterManager.cpp
   KoMainWindow.cpp
   KoApplication.cpp
   KoApplicationAdaptor.cpp
   KoQueryTrader.cpp
   KoFilter.cpp
   KoFilterChain.cpp
   KoPictureKey.cpp
   KoPictureBase.cpp
   KoPicture.cpp
   KoPictureShared.cpp
   KoPictureImage.cpp
   KoPictureClipart.cpp
   KoPictureCollection.cpp
   KoPictureEps.cpp
   KoDocumentInfo.cpp
   KoView.cpp
   KoViewAdaptor.cpp
   KoFrame.cpp
   KoContainerHandler.cpp
   KoEmbeddedDocumentSaver.cpp
   KoDocumentChild.cpp
   KoDocumentInfoDlg.cpp
   KoFactory.cpp
   KoChild.cpp
   kofficeversion.cc
   KoOdfGraphicStyles.cpp
   KoPageFormat.cpp
   KoPageLayout.cpp
   KoFileDialog.cpp
   KoVersionDialog.cpp
   kkbdaccessextensions.cpp
   KoOpenPane.cpp
   KoTemplates.cpp
   KoDetailsPane.cpp
   KoDocumentInfoPropsPage.cpp
   KoTemplatesPane.cpp
   KoRecentDocumentsPane.cpp
    threadAction/ActionJob_p.cpp
    threadAction/KoAction.cpp
    threadAction/KoExecutePolicy.cpp
    threadAction/KoJobsListPolicy.cpp
    KoViewChild.cpp
    KoGridData.cpp
    KoGuidesData.cpp
    KoDockRegistry.cpp
    KoDockWidgetTitleBar.cpp
    KoPrintJob.cpp
    KoExistingDocumentPane.cpp
)


#kde4_add_ui3_files(komain_LIB_SRCS
#   koDocumentInfoUserMetadataWidget.ui )

kde4_add_ui_files( komain_LIB_SRCS
   koDocumentInfoAboutWidget.ui
   koDocumentInfoAuthorWidget.ui
   koOpenPaneBase.ui
   koDetailsPaneBase.ui )

# kde4_add_dcop_skels(komain_LIB_SRCS
#    KoApplicationIface.h
#    KoDocumentIface.h
#    KoViewIface.h
#    KoMainWindowIface.h )

kde4_add_library(komain SHARED ${komain_LIB_SRCS})

target_link_libraries(komain koodf kstore ${KDE4_KPARTS_LIBS} ${KDE4_KFILE_LIBRARY} ${KDE4_KABC_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${KDE4_KDE3SUPPORT_LIBS})

set_target_properties(komain PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS komain ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kodocinfopropspage_PART_SRCS KoDocInfoPropsFactory.cpp )

kde4_add_plugin(kodocinfopropspage ${kodocinfopropspage_PART_SRCS})



target_link_libraries(kodocinfopropspage  ${KDE4_KDECORE_LIBS} komain )

install(TARGETS kodocinfopropspage DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES kodocinfopropspage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES kofficedocker.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES koffice_shell.rc DESTINATION ${DATA_INSTALL_DIR}/koffice )
install( FILES
    KoContainerHandler.h
    KoProperties.h
    KoFilter.h
    KoFilterChain.h
    KoGlobal.h
    KoID.h
    KoGenericRegistry.h
    KoGenericRegistryModel.h
    KoGridData.h
    KoGuidesData.h
    KoDocument.h 
    KoMainWindow.h 
    KoApplication.h 
    KoApplicationAdaptor.h 
    KoQueryTrader.h 
    KoFilterManager.h 
    KoDocumentInfo.h 
    KoView.h 
    KoViewAdaptor.h 
    KoFrame.h 
    KoDocumentChild.h 
    KoDocumentInfoDlg.h 
    KoFactory.h 
    KoChild.h 
    KoPictureKey.h 
    KoPicture.h 
    KoPictureCollection.h 
    kofficeversion.h 
    KoOdfGraphicStyles.h 
    KoPageFormat.h 
    KoPageLayout.h 
    KoVersionDialog.h 
    kkbdaccessextensions.h 
    KoRTree.h
    threadAction/KoJobsListPolicy.h
    threadAction/KoExecutePolicy.h
    threadAction/KoAction.h
    KoDockFactory.h
    KoViewChild.h
    komain_export.h
    KoViewConverter.h
DESTINATION ${INCLUDE_INSTALL_DIR})

