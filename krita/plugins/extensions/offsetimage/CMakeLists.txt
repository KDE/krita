########### next target ###############

set(kritaoffsetimage_SOURCES offsetimage.cpp dlg_offsetimage.cpp kis_offset_processing_visitor.cpp)

ki18n_wrap_ui(kritaoffsetimage_SOURCES wdg_offsetimage.ui )
add_library(kritaoffsetimage MODULE ${kritaoffsetimage_SOURCES})
kcoreaddons_desktop_to_json(kritaoffsetimage kritaoffsetimage.desktop)

target_link_libraries(kritaoffsetimage kritaui)
install(TARGETS kritaoffsetimage  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  offsetimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
