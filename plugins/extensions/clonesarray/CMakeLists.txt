set(kritaclonesarray_SOURCES
  clonesarray.cpp
  dlg_clonesarray.cpp
)

ki18n_wrap_ui(kritaclonesarray_SOURCES wdg_clonesarray.ui )
add_library(kritaclonesarray MODULE ${kritaclonesarray_SOURCES})
target_link_libraries(kritaclonesarray kritaui)
install(TARGETS kritaclonesarray  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES  clonesarray.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
