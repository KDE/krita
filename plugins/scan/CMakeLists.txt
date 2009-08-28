

include_directories( ${KOMAIN_INCLUDES} )


########### next target ###############

set(kofficescan_PART_SRCS scan.cpp )


kde4_add_plugin(kofficescan ${kofficescan_PART_SRCS})



target_link_libraries(kofficescan komain  ${QT_QT3SUPPORT_LIBRARY} )

install(TARGETS kofficescan  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  scan-kpresenter.rc  DESTINATION ${DATA_INSTALL_DIR}/kpresenter/kpartplugins)
install( FILES  scan-krita.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/kpartplugins)
install( FILES  scan-kword.rc  DESTINATION ${DATA_INSTALL_DIR}/kword/kpartplugins)


