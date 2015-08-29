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

add_library(kritahistorydocker MODULE ${kritahistorydocker_PART_SRCS})
kcoreaddons_desktop_to_json(kritahistorydocker kritahistorydocker.desktop)

target_link_libraries(kritahistorydocker kritaimage kritaui)

install(TARGETS kritahistorydocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES kritahistorydocker.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
