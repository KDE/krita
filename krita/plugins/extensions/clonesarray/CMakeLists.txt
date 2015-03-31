########### next target ###############

set(kritaclonesarray_PART_SRCS
  clonesarray.cpp
  dlg_clonesarray.cpp
  )

ki18n_wrap_ui(kritaclonesarray_PART_SRCS wdg_clonesarray.ui )
add_library(kritaclonesarray MODULE ${kritaclonesarray_PART_SRCS})
kcoreaddons_desktop_to_json(kritaclonesarray kritaclonesarray.desktop)

target_link_libraries(kritaclonesarray kritaui)
install(TARGETS kritaclonesarray  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  clonesarray.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaclonesarray.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
