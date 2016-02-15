include_directories(${CMAKE_SOURCE_DIR}/krita/image/metadata)

set(kritametadataeditor_SOURCES metadataeditor.cc kis_entry_editor.cc kis_meta_data_editor.cc kis_meta_data_model.cpp )

ki18n_wrap_ui(kritametadataeditor_SOURCES 
    editors/dublincore.ui
    editors/exif.ui
)

add_library(kritametadataeditor MODULE ${kritametadataeditor_SOURCES})
target_link_libraries(kritametadataeditor kritaui )
install(TARGETS kritametadataeditor  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES metadataeditor.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins )
install( FILES editors/dublincore.rc editors/exif.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins/metadataeditor )
