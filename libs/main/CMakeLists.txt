add_subdirectory( tests )

kde_enable_exceptions()

if (APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
endif ()

include_directories( ${KOMAIN_INCLUDES})

########### next target ###############

if( Qca-qt5_FOUND )
   add_definitions( -DQCA2 )
endif()

set(komain_LIB_SRCS
    KoAutoSaveRecoveryDialog.cpp
    KoApplication.cpp
    KoDockerManager.cpp
    KoDocument.cpp
    KoDocumentEntry.cpp
    KoMainWindow.cpp
    KoPrintingDialog.cpp
    KoPrintJob.cpp
    KoVersionDialog.cpp
    KoView.cpp
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

    KoDetailsPane.cpp
    KoOpenPane.cpp
    KoRecentDocumentsPane.cpp
    KoTemplate.cpp
    KoTemplateCreateDia.cpp
    KoTemplateGroup.cpp
    KoTemplates.cpp
    KoTemplatesPane.cpp
    KoTemplateTree.cpp

    KoFindBase.cpp
    KoFindMatch.cpp
    KoFindText.cpp
    KoFindToolbar.cpp
    KoFindOption.cpp
    KoFindOptionSet.cpp
    KoFindStyle.cpp

    KoPart.cpp

)

if( QT_QTDBUS_FOUND )
    set(komain_LIB_SRCS ${komain_LIB_SRCS}
        KoApplicationAdaptor.cpp
        KoViewAdaptor.cpp
        KoPartAdaptor.cpp
    )
endif()

if (APPLE)
    set(komain_LIB_SRCS ${komain_LIB_SRCS}
        MacSupport.mm
    )
endif ()

ki18n_wrap_ui( komain_LIB_SRCS
   KoOpenPaneBase.ui
   KoDetailsPaneBase.ui
)

QT5_WRAP_CPP(komain_HEADERS_MOC KoDocumentSectionPropertyAction_p.h)

add_library(komain SHARED ${komain_HEADERS_MOC}  ${komain_LIB_SRCS})

target_link_libraries(komain
    kowidgets
    kotextlayout
)

if( KF5_Activities_FOUND )
    target_link_libraries(komain KF5::Activities)
    target_link_libraries(komain LINK_INTERFACE_LIBRARIES  KF5::Activities)
endif()

if (APPLE)
    target_link_libraries(komain ${FOUNDATION_LIBRARY})
endif ()

target_link_libraries(komain LINK_INTERFACE_LIBRARIES kotext kowidgets ${KDE4_KIO_LIBS})

set_target_properties(komain PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS komain ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install(FILES calligradocker.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES calligra_shell.rc DESTINATION ${DATA_INSTALL_DIR}/calligra )

if (APPLE)
    install(FILES osx.stylesheet DESTINATION ${DATA_INSTALL_DIR}/calligra )
endif ()

# For now, don't install the header files because of the conflict with the komvc
# library.
if (FALSE)
install( FILES
    KoApplication.h
    KoDocument.h
    KoDocumentEntry.h
    KoDocumentSectionModel.h
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
    KoMainWindow.h
    KoPageWidgetItem.h
    MacSupport.h
    KoPart.h
    KoPrintJob.h
    KoVersionDialog.h
    KoView.h
    KoStandardAction.h
    komain_export.h
DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)

if( QT_QTDBUS_FOUND )
    install( FILES
        KoApplicationAdaptor.h
        KoViewAdaptor.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)
endif()
endif()

