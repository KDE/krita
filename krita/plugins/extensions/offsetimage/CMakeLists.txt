########### next target ###############

set(kritaoffsetimage_PART_SRCS offsetimage.cpp dlg_offsetimage.cpp kis_offset_processing_visitor.cpp)

kde4_add_ui_files(kritaoffsetimage_PART_SRCS wdg_offsetimage.ui )
kde4_add_plugin(kritaoffsetimage ${kritaoffsetimage_PART_SRCS})

target_link_libraries(kritaoffsetimage kritaui)
install(TARGETS kritaoffsetimage  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  offsetimage.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritaoffsetimage.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
