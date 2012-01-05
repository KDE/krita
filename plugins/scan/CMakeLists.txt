
include_directories( ${KOMAIN_INCLUDES} )


########### next target ###############

set(calligrascan_PART_SRCS scan.cpp )


kde4_add_plugin(calligrascan ${calligrascan_PART_SRCS})

target_link_libraries(calligrascan komain )

install(TARGETS calligrascan  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  scan-stage.rc  DESTINATION ${DATA_INSTALL_DIR}/stage/kpartplugins)
install( FILES  scan-krita.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/kpartplugins)
install( FILES  scan-words.rc  DESTINATION ${DATA_INSTALL_DIR}/words/kpartplugins)


