
set(krita%{APPNAMELC}_PART_SRCS %{APPNAMELC}.cc )


# kde4_add_ui_files(krita%{APPNAMELC}_PART_SRCS myuifile.ui )

kde4_add_plugin(krita%{APPNAMELC} ${krita%{APPNAMELC}_PART_SRCS})

target_link_libraries(krita%{APPNAMELC} kritaui)

install(TARGETS krita%{APPNAMELC}  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  %{APPNAMELC}.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  krita%{APPNAMELC}.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

