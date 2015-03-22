add_subdirectory( tests )

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

if (APPLE)
    find_library(FOUNDATION_LIBRARY Foundation)
endif ()

include_directories( ${KOMAIN_INCLUDES})

########### next target ###############

if( QCA2_FOUND )
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
    config/KoConfigAuthorPage.cpp

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

kde4_add_ui_files( komain_LIB_SRCS
   config/KoConfigAuthorPage.ui
   KoOpenPaneBase.ui
   KoDetailsPaneBase.ui
)
kde4_add_library(komain SHARED ${komain_LIB_SRCS})

target_link_libraries(komain ${extra_libs}
    ${KDE4_KFILE_LIBRARY}
    kowidgets
    kotextlayout
)

if( KActivities_FOUND )
    include_directories(${KACTIVITIES_INCLUDES})
    target_link_libraries(komain ${KACTIVITIES_LIBRARY})
    target_link_libraries(komain LINK_INTERFACE_LIBRARIES  ${KACTIVITIES_LIBRARY})
endif()

if (APPLE)
    target_link_libraries(komain ${FOUNDATION_LIBRARY})
endif ()

target_link_libraries(komain LINK_INTERFACE_LIBRARIES kotext kowidgets )

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

