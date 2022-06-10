set(KRITA_PATTERNDOCKER_SOURCES patterndocker.cpp patterndocker_dock.cpp )
kis_add_library(kritapatterndocker MODULE ${KRITA_PATTERNDOCKER_SOURCES})
target_link_libraries(kritapatterndocker kritaui)
install(TARGETS kritapatterndocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
