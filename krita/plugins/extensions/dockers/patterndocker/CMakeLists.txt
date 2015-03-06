
set(KRITA_PATTERNDOCKER_PART_SRCS patterndocker.cpp patterndocker_dock.cpp )

add_library(kritapatterndocker MODULE ${KRITA_PATTERNDOCKER_PART_SRCS})

target_link_libraries(kritapatterndocker kritaui)

install(TARGETS kritapatterndocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_patterndocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

