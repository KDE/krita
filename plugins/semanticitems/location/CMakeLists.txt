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
    ki18n_wrap_ui( calligra_semanticitem_location_LIB_SRCS
        KoRdfLocationEditWidgetMarble.ui
        KoRdfLocationViewWidgetMarble.ui
    )
else()
    ki18n_wrap_ui( calligra_semanticitem_location_LIB_SRCS
        KoRdfLocationEditWidget.ui
        KoRdfLocationViewWidget.ui
    )
endif()

add_library(calligra_semanticitem_location MODULE ${calligra_semanticitem_location_LIB_SRCS})

if( MARBLE_FOUND )
include_directories(${MARBLE_INCLUDE_DIR})
target_link_libraries(calligra_semanticitem_location ${MARBLE_LIBRARIES})
target_link_libraries(calligra_semanticitem_location LINK_INTERFACE_LIBRARIES  ${MARBLE_LIBRARIES})
endif()

target_link_libraries(calligra_semanticitem_location   ${KORDF_LIBS} KF5::KDELibs4Support)
kcoreaddons_desktop_to_json(calligra_semanticitem_location calligra_semanticitem_location.desktop)

install(TARGETS calligra_semanticitem_location  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
