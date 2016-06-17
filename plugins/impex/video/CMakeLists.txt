add_subdirectory(tests)

# export

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

set(kritavideoexport_SOURCES
    kis_video_export.cpp
    video_saver.cpp
    video_export_options_dialog.cpp
    )

ki18n_wrap_ui(kritavideoexport_SOURCES video_export_options_dialog.ui )

add_library(kritavideoexport MODULE ${kritavideoexport_SOURCES})

generate_export_header(kritavideoexport BASE_NAME kritavideoexport)

target_link_libraries(kritavideoexport kritaui)

install(TARGETS kritavideoexport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
