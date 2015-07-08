add_subdirectory( tests )

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

include_directories( ${KORDF_INCLUDES})

########### next target ###############

set(kordf_LIB_SRCS
    KoDocumentRdf.cpp
    KoDocumentRdfEditWidget.cpp
    KoSopranoTableModelDelegate.cpp
    KoSopranoTableModel.cpp
    KoRdfPrefixMapping.cpp
    KoSemanticStylesheet.cpp
    KoRdfSemanticItem.cpp
    KoRdfSemanticItemFactoryBase.cpp
    KoRdfSemanticItemRegistry.cpp
    KoRdfSemanticItemViewSite.cpp
    KoRdfSemanticTree.cpp
    KoRdfSemanticTreeWidgetItem.cpp
    KoSemanticStylesheetsEditor.cpp
    KoChangeTrackerDisabledRAII.cpp
    RdfSemanticTreeWidgetAction.cpp
    RdfSemanticTreeWidgetSelectAction.cpp
    InsertSemanticObjectActionBase.cpp
    InsertSemanticObjectCreateAction.cpp
    InsertSemanticObjectReferenceAction.cpp
)

kde4_add_ui_files( kordf_LIB_SRCS
   KoDocumentRdfEditWidget.ui
   KoSemanticStylesheetsEditor.ui
)

kde4_add_library(kordf SHARED ${kordf_LIB_SRCS})

target_link_libraries(kordf komain )
target_link_libraries(kordf ${SOPRANO_LIBRARIES})
target_link_libraries(kordf LINK_INTERFACE_LIBRARIES  ${SOPRANO_LIBRARIES})

target_link_libraries(kordf LINK_INTERFACE_LIBRARIES komain )

set_target_properties(kordf PROPERTIES
VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kordf ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES calligra_semanticitem.desktop  DESTINATION ${SERVICETYPES_INSTALL_DIR})

install( FILES
    kordf_export.h
    RdfForward.h
    KoDocumentRdf.h
    KoDocumentRdfEditWidget.h
    KoRdfPrefixMapping.h
    KoRdfSemanticItem.h
    KoRdfSemanticItemFactoryBase.h
    KoRdfSemanticItemRegistry.h
    KoRdfSemanticItemViewSite.h
    KoRdfSemanticTree.h
    KoRdfSemanticTreeWidgetItem.h
    KoSemanticStylesheet.h
    KoSemanticStylesheetsEditor.h
    RdfSemanticTreeWidgetAction.h
    RdfSemanticTreeWidgetSelectAction.h
DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)
