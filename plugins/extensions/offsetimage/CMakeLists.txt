set(kritaoffsetimage_SOURCES 
    offsetimage.cpp 
    dlg_offsetimage.cpp       
    kis_offset_processing_visitor.cpp
)

ki18n_wrap_ui(kritaoffsetimage_SOURCES wdg_offsetimage.ui )
add_library(kritaoffsetimage MODULE ${kritaoffsetimage_SOURCES})
target_link_libraries(kritaoffsetimage kritaui)
install(TARGETS kritaoffsetimage  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
