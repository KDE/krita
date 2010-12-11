include_directories( ${KOMAIN_INCLUDES} ${KOTEXT_INCLUDES} )

set(kofficehistorydocker_PART_SRCS
    History.h
    History.cpp
    HistoryDock.h
    HistoryDock.cpp
    KisUndoView.cpp
    KisUndoView.h
    KisUndoModel.h
    KisUndoModel.cpp
)

kde4_add_plugin(kofficehistorydocker ${kofficehistorydocker_PART_SRCS})

target_link_libraries(kofficehistorydocker komain kritaimage kritaui)

install(TARGETS kofficehistorydocker  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficehistorydocker.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
