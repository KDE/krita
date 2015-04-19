include_directories(
    ${KORDF_INCLUDES}
)
set(calligra_semanticitem_location_LIB_SRCS
    KoLocationSemanticItemFactory.cpp
    KoLocationSemanticItemPlugin.cpp
    KoRdfLocation.cpp
    KoRdfLocationEditWidget.cpp
    KoRdfLocationTreeWidgetItem.cpp
)

if(MARBLE_FOUND)
    kde4_add_ui_files( calligra_semanticitem_location_LIB_SRCS
        KoRdfLocationEditWidgetMarble.ui
        KoRdfLocationViewWidgetMarble.ui
    )
else()
    kde4_add_ui_files( calligra_semanticitem_location_LIB_SRCS
        KoRdfLocationEditWidget.ui
        KoRdfLocationViewWidget.ui
    )
endif()

kde4_add_plugin(calligra_semanticitem_location ${calligra_semanticitem_location_LIB_SRCS})

if( MARBLE_FOUND )
include_directories(${MARBLE_INCLUDE_DIR})
target_link_libraries(calligra_semanticitem_location ${MARBLE_LIBRARIES})
target_link_libraries(calligra_semanticitem_location LINK_INTERFACE_LIBRARIES  ${MARBLE_LIBRARIES})
endif()

target_link_libraries(calligra_semanticitem_location ${KDE4_KDECORE_LIBS}  ${KORDF_LIBS})


install(TARGETS calligra_semanticitem_location  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   calligra_semanticitem_location.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
