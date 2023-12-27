add_subdirectory(brush/tests)

include_directories(brush
                    duplicate
                    ${CMAKE_CURRENT_BINARY_DIR})

set(kritadefaultpaintops_SOURCES
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
        duplicate/KisDuplicateOptionData.cpp
        duplicate/KisDuplicateOptionModel.cpp
        duplicate/KisDuplicateOptionWidget.cpp
    )

ki18n_wrap_ui(kritadefaultpaintops_SOURCES duplicate/wdgduplicateop.ui )

kis_add_library(kritadefaultpaintops_static STATIC ${kritadefaultpaintops_SOURCES})
generate_export_header(kritadefaultpaintops_static BASE_NAME kritadefaultpaintops EXPORT_MACRO_NAME KRITADEFAULTPAINTOPS_EXPORT)

target_link_libraries(kritadefaultpaintops_static kritalibpaintop)

kis_add_library(kritadefaultpaintops MODULE defaultpaintops_plugin.cc)

target_link_libraries(kritadefaultpaintops kritadefaultpaintops_static)

install(TARGETS kritadefaultpaintops  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-paintbrush.png
    krita-eraser.png
    krita-duplicate.png
     DESTINATION ${DATA_INSTALL_DIR}/krita/images)
