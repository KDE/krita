include_directories(
    ${KORDF_INCLUDES}
)
set(calligra_semanticitem_event_LIB_SRCS
    KoEventSemanticItemFactory.cpp
    KoEventSemanticItemPlugin.cpp
    KoRdfCalendarEvent.cpp
    KoRdfCalendarEventTreeWidgetItem.cpp
)

ki18n_wrap_ui( calligra_semanticitem_event_LIB_SRCS
   KoRdfCalendarEventEditWidget.ui
)

add_library(calligra_semanticitem_event MODULE ${calligra_semanticitem_event_LIB_SRCS})

if (KF5CalendarCore_FOUND AND KF5Akonadi_FOUND)
   add_definitions(-DKDEPIMLIBS_FOUND)
   target_link_libraries(calligra_semanticitem_event
      KF5::CalendarCore
      KF5::AkonadiCore
   )
endif ()

target_link_libraries(calligra_semanticitem_event   ${KORDF_LIBS})
kcoreaddons_desktop_to_json(calligra_semanticitem_event calligra_semanticitem_event.desktop)

install(TARGETS calligra_semanticitem_event  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
