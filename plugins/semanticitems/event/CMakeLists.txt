include_directories(
    ${KORDF_INCLUDES}
)
set(calligra_semanticitem_event_LIB_SRCS
    KoEventSemanticItemFactory.cpp
    KoEventSemanticItemPlugin.cpp
    KoRdfCalendarEvent.cpp
    KoRdfCalendarEventTreeWidgetItem.cpp
)

kde4_add_ui_files( calligra_semanticitem_event_LIB_SRCS
   KoRdfCalendarEventEditWidget.ui
)

kde4_add_plugin(calligra_semanticitem_event ${calligra_semanticitem_event_LIB_SRCS})

if (KDEPIMLIBS_FOUND)
   include_directories(${KDEPIMLIBS_INCLUDE_DIR})
   add_definitions(-DKDEPIMLIBS_FOUND)
   target_link_libraries(calligra_semanticitem_event
      ${KDEPIMLIBS_KCALCORE_LIBS}
      ${KDEPIMLIBS_AKONADI_LIBS}
   )
endif ()

target_link_libraries(calligra_semanticitem_event ${KDE4_KDECORE_LIBS}  ${KORDF_LIBS})


install(TARGETS calligra_semanticitem_event  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES   calligra_semanticitem_event.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
