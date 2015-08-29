include_directories(${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(kritametadataeditor_PART_SRCS metadataeditor.cc kis_entry_editor.cc kis_meta_data_editor.cc kis_meta_data_model.cpp )

ki18n_wrap_ui(kritametadataeditor_PART_SRCS 
    editors/dublincore.ui
    editors/exif.ui
)

add_library(kritametadataeditor MODULE ${kritametadataeditor_PART_SRCS})
kcoreaddons_desktop_to_json(kritametadataeditor kritametadataeditor.desktop)

target_link_libraries(kritametadataeditor kritaui )

install(TARGETS kritametadataeditor  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES metadataeditor.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins )
install( FILES kritametadataeditor.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
install( FILES editors/dublincore.rc editors/exif.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins/metadataeditor )
