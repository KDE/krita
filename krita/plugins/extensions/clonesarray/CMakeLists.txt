########### next target ###############

set(kritaclonesarray_PART_SRCS
  clonesarray.cpp
  dlg_clonesarray.cpp
  )

kde4_add_ui_files(kritaclonesarray_PART_SRCS wdg_clonesarray.ui )
kde4_add_plugin(kritaclonesarray ${kritaclonesarray_PART_SRCS})

target_link_libraries(kritaclonesarray kritaui)
install(TARGETS kritaclonesarray  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  clonesarray.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaclonesarray.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
