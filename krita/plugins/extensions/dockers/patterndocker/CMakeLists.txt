
set(KRITA_PATTERNDOCKER_SOURCES patterndocker.cpp patterndocker_dock.cpp )

add_library(kritapatterndocker MODULE ${KRITA_PATTERNDOCKER_SOURCES})
kcoreaddons_desktop_to_json(kritapatterndocker krita_patterndocker.desktop)

target_link_libraries(kritapatterndocker kritaui)

install(TARGETS kritapatterndocker  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
