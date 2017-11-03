add_subdirectory(brush/tests)

include_directories(brush
                    duplicate
                    ${CMAKE_CURRENT_BINARY_DIR})

set(kritadefaultpaintops_SOURCES
	defaultpaintops_plugin.cc
	brush/kis_brushop.cpp
        brush/KisBrushOpResources.cpp
        brush/KisBrushOpSettings.cpp
	brush/kis_brushop_settings_widget.cpp
        brush/KisDabRenderingQueue.cpp
        brush/KisDabRenderingQueueCache.cpp
        brush/KisDabRenderingJob.cpp
        brush/KisDabRenderingExecutor.cpp
        duplicate/kis_duplicateop.cpp
	duplicate/kis_duplicateop_settings.cpp
	duplicate/kis_duplicateop_settings_widget.cpp
	duplicate/kis_duplicateop_option.cpp
    )

ki18n_wrap_ui(kritadefaultpaintops_SOURCES duplicate/wdgduplicateop.ui )

add_library(kritadefaultpaintops MODULE ${kritadefaultpaintops_SOURCES})
generate_export_header(kritadefaultpaintops BASE_NAME kritadefaultpaintops EXPORT_MACRO_NAME KRITADEFAULTPAINTOPS_EXPORT)

target_link_libraries(kritadefaultpaintops kritalibpaintop)

install(TARGETS kritadefaultpaintops  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-paintbrush.png
    krita-eraser.png
    krita-duplicate.png
     DESTINATION ${DATA_INSTALL_DIR}/krita/images)
