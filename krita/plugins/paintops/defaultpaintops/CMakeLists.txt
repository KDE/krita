add_subdirectory(brush/tests)

include_directories(brush)
include_directories(pen)
include_directories(eraser)
include_directories(duplicate)


set(kritadefaultpaintops_PART_SRCS
	defaultpaintops_plugin.cc
	brush/kis_brushop.cpp
	brush/kis_brushop_settings_widget.cpp
	duplicate/kis_duplicateop.cpp
	duplicate/kis_duplicateop_settings.cpp
	duplicate/kis_duplicateop_settings_widget.cpp
	duplicate/kis_duplicateop_option.cpp
    )

ki18n_wrap_ui(kritadefaultpaintops_PART_SRCS duplicate/wdgduplicateop.ui )

add_library(kritadefaultpaintops MODULE ${kritadefaultpaintops_PART_SRCS})



target_link_libraries(kritadefaultpaintops kritalibpaintop)
kcoreaddons_desktop_to_json(kritadefaultpaintops kritadefaultpaintops.desktop)

install(TARGETS kritadefaultpaintops  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-paintbrush.png
    krita-eraser.png
    krita-duplicate.png
     DESTINATION ${DATA_INSTALL_DIR}/krita/images)
