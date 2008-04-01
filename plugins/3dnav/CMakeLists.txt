
set(kritaspacenav_PART_SRCS SpaceNavigatorPlugin.cpp )

# kde4_add_ui_files(kritaspacenav_PART_SRCS myuifile.ui )

kde4_add_plugin(kritaspacenav ${kritaspacenav_PART_SRCS})

target_link_libraries(kritaspacenav kritaui spnav flake ${KRITA_UI_LIBS} ${KOFFICE_LIBS})

install(TARGETS kritaspacenav  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  spacenav.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaspacenav.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

