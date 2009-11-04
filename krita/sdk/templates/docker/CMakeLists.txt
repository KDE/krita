
set(KRITA_%{APPNAMEUC}_PART_SRCS %{APPNAMELC}.cc %{APPNAMELC}_dock.cc )

# kde4_add_ui_files(krita%{APPNAMELC}_PART_SRCS myuifile.ui )

kde4_add_plugin(krita%{APPNAMELC} ${KRITA_%{APPNAMEUC}_PART_SRCS})

target_link_libraries(krita%{APPNAMELC} kritaui)

install(TARGETS krita%{APPNAMELC}  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_%{APPNAMELC}.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

