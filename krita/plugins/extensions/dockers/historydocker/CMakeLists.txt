include_directories( ${KRITA_INCLUDES} ${KOTEXT_INCLUDES} )

set(kritahistorydocker_PART_SRCS
    History.h
    History.cpp
    HistoryDock.h
    HistoryDock.cpp
    KisUndoView.cpp
    KisUndoView.h
    KisUndoModel.h
    KisUndoModel.cpp
)

kde4_add_plugin(kritahistorydocker ${kritahistorydocker_PART_SRCS})

target_link_libraries(kritahistorydocker kritaimage kritaui)

install(TARGETS kritahistorydocker  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kritahistorydocker.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
