

add_subdirectory( tests )

include_directories( ${KWMF_INCLUDES} ${KOSTORE_INCLUDES} )

########### next target ###############

set(kofficecore_LIB_SRCS
   KoDocument.cpp
   KoDocumentAdaptor.cpp
   KoProperties.cpp
   KoGlobal.cpp
   KoUnit.cpp
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
   KoPictureWmf.cpp
   KoDocumentInfo.cpp
   KoView.cpp
   KoViewAdaptor.cpp
   KoFrame.cpp
   KoContainerHandler.cpp
   KoDocumentChild.cpp
   KoDocumentInfoDlg.cpp
   KoFactory.cpp
   KoChild.cpp
   kofficeversion.cc
   KoOasisStyles.cpp
   KoStyleStack.cpp
   KoGenStyles.cpp
   KoGenStyle.cpp
   KoOasisSettings.cpp
   KoPageLayout.cpp
   KoFileDialog.cpp
   KoXmlNS.cpp
   KoDom.cpp
   KoVersionDialog.cpp
   KoOasisStore.cpp
   kkbdaccessextensions.cpp
   KoOpenPane.cpp
   KoTemplates.cpp
   KoDetailsPane.cpp
   KoOasisLoadingContext.cpp
   KoRect.cpp
   KoDocumentInfoPropsPage.cpp
   KoTemplatesPane.cpp
   KoRecentDocumentsPane.cpp
)

kde4_automoc(${kofficecore_LIB_SRCS})

#kde4_add_ui3_files(kofficecore_LIB_SRCS
#   koDocumentInfoUserMetadataWidget.ui )

kde4_add_ui_files( kofficecore_LIB_SRCS
   koDocumentInfoAboutWidget.ui
   koDocumentInfoAuthorWidget.ui
   koOpenPaneBase.ui
   koDetailsPaneBase.ui )

# kde4_add_dcop_skels(kofficecore_LIB_SRCS
#    KoApplicationIface.h
#    KoDocumentIface.h
#    KoViewIface.h
#    KoMainWindowIface.h )

kde4_add_library(kofficecore SHARED ${kofficecore_LIB_SRCS})

target_link_libraries(kofficecore  ${KDE4_KPARTS_LIBS} kstore ${KDE4_KABC_LIBS} ${KDE4_KDEPRINT_LIBS} kowmf)

set_target_properties(kofficecore PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )
install(TARGETS kofficecore DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

set(kodocinfopropspage_PART_SRCS KoDocInfoPropsFactory.cpp )

kde4_automoc(${kodocinfopropspage_PART_SRCS})

kde4_add_plugin(kodocinfopropspage ${kodocinfopropspage_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kodocinfopropspage )

target_link_libraries(kodocinfopropspage  ${KDE4_KDECORE_LIBS} kofficecore )

install(TARGETS kodocinfopropspage DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES kodocinfopropspage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES koffice_shell.rc DESTINATION ${DATA_INSTALL_DIR}/koffice )
install( FILES
    KoContainerHandler.h
    KoProperties.h
    KoFilter.h
    KoFilterChain.h
    KoGlobal.h
    KoUnit.h
    KoID.h
    KoGenericRegistry.h
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
    KoOasisStyles.h 
    KoStyleStack.h 
    KoGenStyles.h 
    KoOasisSettings.h 
    KoPageLayout.h 
    KoXmlNS.h 
    KoDom.h 
    KoVersionDialog.h 
    kkbdaccessextensions.h 
    KoOpenPane.h 
    KoOasisLoadingContext.h 
    KoPoint.h 
    KoRTree.h
    KoGenStyle.h DESTINATION ${INCLUDE_INSTALL_DIR})

