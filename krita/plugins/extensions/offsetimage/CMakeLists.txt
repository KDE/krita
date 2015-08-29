########### next target ###############

set(kritaoffsetimage_PART_SRCS offsetimage.cpp dlg_offsetimage.cpp kis_offset_processing_visitor.cpp)

ki18n_wrap_ui(kritaoffsetimage_PART_SRCS wdg_offsetimage.ui )
add_library(kritaoffsetimage MODULE ${kritaoffsetimage_PART_SRCS})
kcoreaddons_desktop_to_json(kritaoffsetimage kritaoffsetimage.desktop)

target_link_libraries(kritaoffsetimage kritaui)
install(TARGETS kritaoffsetimage  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  offsetimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaoffsetimage.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
