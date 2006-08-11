

include_directories( ${KOFFICECORE_INCLUDES} )


########### next target ###############

set(kofficescan_PART_SRCS scan.cpp )

kde4_automoc(${kofficescan_PART_SRCS})

kde4_add_plugin(kofficescan ${kofficescan_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kofficescan )

target_link_libraries(kofficescan  ${KDE4_KPARTS_LIBS} ${KDE4_KIO_LIBS} kofficecore )

install(TARGETS kofficescan  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  scan-kpresenter.rc  DESTINATION ${DATA_INSTALL_DIR}/kpresenter/kpartplugins)
install( FILES  scan-krita.rc  DESTINATION ${DATA_INSTALL_DIR}/krita/kpartplugins)
install( FILES  scan-kword.rc  DESTINATION ${DATA_INSTALL_DIR}/kword/kpartplugins)


