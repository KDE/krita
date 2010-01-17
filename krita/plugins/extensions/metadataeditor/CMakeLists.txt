include_directories(${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(kritametadataeditor_PART_SRCS metadataeditor.cc kis_entry_editor.cc kis_meta_data_editor.cc kis_meta_data_model.cpp )

# kde4_add_ui_files(kritametadataeditor_PART_SRCS myuifile.ui )

kde4_add_plugin(kritametadataeditor ${kritametadataeditor_PART_SRCS})

target_link_libraries(kritametadataeditor kritaui ${QT_QTUITOOLS_LIBRARY} )

install(TARGETS kritametadataeditor  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES metadataeditor.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins )
install( FILES kritametadataeditor.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES editors/dublincore.ui editors/dublincore.rc editors/exif.ui editors/exif.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins/metadataeditor )
