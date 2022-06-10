set(kritahistorydocker_SOURCES
    History.h
    History.cpp
    HistoryDock.h
    HistoryDock.cpp
    KisUndoView.cpp
    KisUndoView.h
    KisUndoModel.h
    KisUndoModel.cpp
    DlgConfigureHistoryDock.cpp
)

kis_add_library(kritahistorydocker MODULE ${kritahistorydocker_SOURCES})
target_link_libraries(kritahistorydocker kritaimage kritaui)
install(TARGETS kritahistorydocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
