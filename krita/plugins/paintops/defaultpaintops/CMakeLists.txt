add_subdirectory(brush/tests)

include_directories(brush)
include_directories(pen)
include_directories(eraser)
include_directories(duplicate)


set(kritadefaultpaintops_SOURCES
	defaultpaintops_plugin.cc
	brush/kis_brushop.cpp
	brush/kis_brushop_settings_widget.cpp
	duplicate/kis_duplicateop.cpp
	duplicate/kis_duplicateop_settings.cpp
	duplicate/kis_duplicateop_settings_widget.cpp
	duplicate/kis_duplicateop_option.cpp
    )

ki18n_wrap_ui(kritadefaultpaintops_SOURCES duplicate/wdgduplicateop.ui )

add_library(kritadefaultpaintops MODULE ${kritadefaultpaintops_SOURCES})

target_link_libraries(kritadefaultpaintops kritalibpaintop)

install(TARGETS kritadefaultpaintops  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-paintbrush.png
    krita-eraser.png
    krita-duplicate.png
     DESTINATION ${DATA_INSTALL_DIR}/krita/images)
