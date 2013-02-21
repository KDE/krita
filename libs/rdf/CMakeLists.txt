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
    KoRdfFoaF.cpp
    KoRdfCalendarEvent.cpp
    KoRdfLocationEditWidget.cpp
    KoRdfLocation.cpp
    KoRdfSemanticItemViewSite.cpp
    KoRdfSemanticTree.cpp
    KoRdfSemanticTreeWidgetItem.cpp
    KoRdfFoaFTreeWidgetItem.cpp
    KoRdfCalendarEventTreeWidgetItem.cpp
    KoRdfLocationTreeWidgetItem.cpp
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
   KoRdfFoaFEditWidget.ui
   KoRdfCalendarEventEditWidget.ui
   KoSemanticStylesheetsEditor.ui
)

#    if(MARBLE_FOUND)
#        kde4_add_ui_files( kordf_LIB_SRCS
#           KoRdfLocationEditWidgetMarble.ui
#           KoRdfLocationViewWidgetMarble.ui
#        )
#    else(MARBLE_FOUND)
    kde4_add_ui_files( kordf_LIB_SRCS
       KoRdfLocationEditWidget.ui
       KoRdfLocationViewWidget.ui
    )
#    endif(MARBLE_FOUND)

kde4_add_library(kordf SHARED ${kordf_LIB_SRCS})

target_link_libraries(kordf ${extra_libs} komain )
target_link_libraries(kordf ${SOPRANO_LIBRARIES})
target_link_libraries(kordf LINK_INTERFACE_LIBRARIES  ${SOPRANO_LIBRARIES})

if( MARBLE_FOUND )
include_directories(${MARBLE_INCLUDE_DIR})
target_link_libraries(kordf ${MARBLE_LIBRARIES})
target_link_libraries(kordf LINK_INTERFACE_LIBRARIES  ${MARBLE_LIBRARIES})
endif( MARBLE_FOUND )

target_link_libraries(kordf LINK_INTERFACE_LIBRARIES komain ${KDE4_KPARTS_LIBS})

set_target_properties(kordf PROPERTIES
VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kordf ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES
    kordf_export.h
    KoRdfFoaF.h
    KoRdfCalendarEvent.h
    KoRdfLocation.h
    KoRdfFoaFTreeWidgetItem.h
    KoRdfCalendarEventTreeWidgetItem.h
    KoRdfLocationTreeWidgetItem.h
    KoChangeTrackerDisabledRAII.h
    RdfForward.h
    KoDocumentRdf.h
    KoSopranoTableModelDelegate.h
    KoSopranoTableModel.h
    KoSemanticStylesheet.h
    KoRdfSemanticItem.h
    KoRdfSemanticItemViewSite.h
    KoRdfSemanticTree.h
    KoRdfSemanticTreeWidgetItem.h
    RdfSemanticTreeWidgetAction.h
    RdfSemanticTreeWidgetSelectAction.h
    InsertSemanticObjectActionBase.h
    InsertSemanticObjectCreateAction.h
    InsertSemanticObjectReferenceAction.h
    KoSemanticStylesheetsEditor.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

